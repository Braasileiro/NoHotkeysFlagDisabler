#include "pch.h"
#include "logger.hpp"

// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices
static BOOL(WINAPI* _RegisterRawInputDevices)(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize) = RegisterRawInputDevices;

// Hook
BOOL WINAPI HK_RegisterRawInputDevices(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize)
{
    if (cbSize != sizeof(RAWINPUTDEVICE))
    {
        logger.Log(WARNING, "RegisterRawInputDevices struct size not matches. Running with the original parameters.");

        return _RegisterRawInputDevices(pRawInputDevices, uiNumDevices, cbSize);
    }

    std::vector<RAWINPUTDEVICE> devices;

    for (size_t i = 0; i < uiNumDevices; i++)
    {
        auto current = pRawInputDevices[i];

        // Check if the device has a RIDEV_NOHOTKEYS flag
        if (current.dwFlags & RIDEV_NOHOTKEYS)
        {
            // Override with the default (0)
            current.dwFlags = 0;

            // TODO: Log here
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
        logger.Log(EXCEPTION, "DetourTransactionCommit returned ERROR_INVALID_DATA.");
        return FALSE;
    case ERROR_INVALID_OPERATION:
        logger.Log(EXCEPTION, "DetourTransactionCommit returned ERROR_INVALID_OPERATION.");
        return FALSE;
    default:
        logger.Log(EXCEPTION, "DetourTransactionCommit returned an unknown error.");
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
        logger.Log(INFO, "RegisterRawInputDevices hook attached.");
    }
}

void Detach()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)_RegisterRawInputDevices, HK_RegisterRawInputDevices);

    if (Commit())
    {
        logger.Log(INFO, "RegisterRawInputDevices hook detached.");
    }
}

// Entry
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Attach();
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
