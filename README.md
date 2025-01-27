# NoHotkeysFlagDisabler
### The development of this project has been moved to [Win32Hooks](https://github.com/Braasileiro/Win32Hooks).

Disable [**RIDEV_NOHOTKEYS**](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawinputdevice) flag on RawInput devices.

You know that game that blocks any function of the **WIN** key and only minimizes with ALT+TAB? Or worse, if you don't have any other windows open, ALT+TAB is no longer a solution and you end up pressing CTRL+ALT+DEL to be able to minimize the game in Borderless or Fullscreen.

I came across this problem in [Mega Man Zero/ZX Legacy Collection](https://store.steampowered.com/app/999020/Mega_Man_ZeroZX_Legacy_Collection) recently and discovered that the cause is the [RIDEV_NOHOTKEYS](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawinputdevice) flag that is passed to the device when registering with RawInput. Thanks to [SpecialK](https://github.com/SpecialKO/SpecialK) for showing me this in the logs.

I developed this solution using [Detours](https://github.com/microsoft/Detours) to be injected into the application process. It has a hook in the [RegisterRawInputDevices](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices) function to check if the device being registered has the [RIDEV_NOHOTKEYS](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawinputdevice) flag and, if found, replaces it with **RIDEV_NOLEGACY**. In addition to returning control of the **WIN** key to the user, this flag prevents double input in some applications that use RawInput and LegacyInput together.

# Usage
This program has been designed so that it is **injected** into the application process. You can use any **DLL injector**, here are some examples.

### [SpecialK](https://github.com/SpecialKO/SpecialK)
You can use SpecialK [Custom Plugin](https://wiki.special-k.info/en/SpecialK/Tools#custom-plugin) feature to load the DLL along with it. I recommend setting **'When'** configuration to **'Early'** because SpecialK itself has a hook in the **RegisterRawInputDevices** function, so in order to avoid problems, loading it in **'Early'** makes SpecialK hook the function previously modified by the NoHotkeysFlagDisabler.

### [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)
A powerful multipurpose injector that's easy to configure.

Download the latest version taking into account whether the application is **x86** or **x64**, rename the Ultimate ASI Loader DLL to something that the application loads, most games use **'dinput8.dll'**. Place the **NoHotkeysFlagDisabler(32|64).dll** inside the **'plugins'** or **'scripts'** folder.

This folder has to be in the **same directory as the Ultimate ASI Loader**, you should probably create it the first time you set it up in your application, I usually use the name **'plugins'**.

The Ultimate ASI Loader only reads files with the **.asi** extension, but these are only .dll files with the extension renamed.

Rename the extension of **NoHotkeysFlagDisabler(32|64).dll** to **NoHotkeysFlagDisabler(32|64).asi**.

# Notes
I came across this problem in a specific game and decided to create this solution. I realized that it could be used in other applications and decided to publish it here.

I don't intend to add other features if they aren't related to the function used in the hook, so I'll leave the issues closed.

I hope this helps other people who have had the same problem as me. For me, no game should block keyboard shortcuts native to the operating system. If someone wants to disable the WIN key, they can do it themselves with the keyboard software or by other means, but a game shouldn't force this.
