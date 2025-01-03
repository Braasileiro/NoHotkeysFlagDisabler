#include "pch.h"
#include "logger.h"

// States
int kNotified = 0;

// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices
static BOOL(WINAPI* _RegisterRawInputDevices)(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize) = RegisterRawInputDevices;

// Hook
BOOL WINAPI HK_RegisterRawInputDevices(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize)
{
    if (cbSize != sizeof(RAWINPUTDEVICE))
    {
        spdlog::warn("RegisterRawInputDevices struct size not matches. Running with the original parameters.");

        return _RegisterRawInputDevices(pRawInputDevices, uiNumDevices, cbSize);
    }

    std::vector<RAWINPUTDEVICE> devices;

    for (size_t i = 0; i < uiNumDevices; i++)
    {
        auto current = pRawInputDevices[i];

        // Check if the device has a RIDEV_NOHOTKEYS flag
        if (current.dwFlags & RIDEV_NOHOTKEYS)
        {
            /*
             * Override with RIDEV_NOLEGACY. Prevents RawInput from sending 'WM_' notifications.
             * This flag prevents double input in some applications that use RawInput and LegacyInput together.
             * ReShade is affected by this in games that use both APIs.
             */
            current.dwFlags = RIDEV_NOLEGACY;

            // TODO: External configuration for the user to be able to configure the flags they want?

            /*
             * Maximum of 5 notifications.
             * Some applications frequently unregister and register RawInput devices.
             */
            if (kNotified < 5)
            {
                spdlog::info("The device has RIDEV_NOHOTKEYS flag. Replacing with RIDEV_NOLEGACY...");

                kNotified += 1;

                if (kNotified == 5)
                {
                    spdlog::info("Flag override messages on devices will no longer be notified.");
                }
            }
        }

        devices.push_back(current);
    }

    return _RegisterRawInputDevices(
        devices.data(),
        static_cast<UINT>(devices.size()),
        cbSize
    );
}

// Detouring
BOOL Commit()
{
    auto result = DetourTransactionCommit();

    switch (result)
    {
    case NO_ERROR:
        return TRUE;
    case ERROR_INVALID_DATA:
        spdlog::error("DetourTransactionCommit returned ERROR_INVALID_DATA.");
        return FALSE;
    case ERROR_INVALID_OPERATION:
        spdlog::error("DetourTransactionCommit returned ERROR_INVALID_OPERATION.");
        return FALSE;
    default:
        spdlog::error("DetourTransactionCommit returned an unknown error.");
        return FALSE;
    }
}

void Attach()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)_RegisterRawInputDevices, HK_RegisterRawInputDevices);

    if (Commit())
    {
        spdlog::info("RegisterRawInputDevices hook attached.");
    }
}

void Detach()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)_RegisterRawInputDevices, HK_RegisterRawInputDevices);

    if (Commit())
    {
        spdlog::info("RegisterRawInputDevices hook detached.");
    }
}

void Init()
{
    Logger::Init();

    Attach();
}

// Entry
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Init();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        Detach();
        break;
    }

    return TRUE;
}
