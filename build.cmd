@echo off
setlocal

where cl >nul 2>nul
if errorlevel 1 (
    echo ERROR: MSVC cl.exe was not found in PATH.
    echo Run build.cmd from a Command Prompt configured for the MSVC Build Tools.
    exit /b 1
)

if not exist build\windows mkdir build\windows
if not exist build\windows\modules mkdir build\windows\modules

cl /nologo /std:c17 /W4 /TC /Iinclude /Fo"build\windows\\" /Fe"build\windows\rictus.exe" src\main.c src\rictus.c src\module_inventory.c src\module_lifecycle.c ..\ABI\src\module.c ..\ABI\src\module_registry.c platforms\windows\rictus_module_loader_windows.c /link
if errorlevel 1 exit /b 1

cl /nologo /std:c17 /W4 /TC /Iinclude /Imodules\irc\include /LD modules\irc\src\irc_module.c src\config.c src\irc.c src\irc_message.c src\event.c src\dispatch.c src\command.c platforms\windows\rictus_net_windows.c platforms\windows\rictus_tls_windows.c /Fe"build\windows\modules\irc.dll" /link ws2_32.lib secur32.lib crypt32.lib
if errorlevel 1 exit /b 1

echo BUILD SUCCESSFUL
exit /b 0
