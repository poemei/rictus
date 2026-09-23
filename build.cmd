@echo off
setlocal

where cl >nul 2>nul
if errorlevel 1 (
    echo ERROR: MSVC cl.exe was not found in PATH.
    echo Run build.cmd from a Command Prompt configured for the MSVC Build Tools.
    exit /b 1
)

if not exist build\windows mkdir build\windows

cl /nologo /std:c17 /W4 /TC /Iinclude /Fo"build\windows\\" /Fe"build\windows\rictus.exe" src\main.c src\rictus.c
if errorlevel 1 exit /b 1

echo BUILD SUCCESSFUL
exit /b 0
