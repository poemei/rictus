@echo off
setlocal

where cl >nul 2>nul
if errorlevel 1 (
    echo ERROR: MSVC cl.exe was not found in PATH.
    echo Run build.cmd from a Command Prompt configured for the MSVC Build Tools.
    exit /b 1
)

if "%OPENSSL_ROOT_DIR%"=="" (
    echo ERROR: OPENSSL_ROOT_DIR is not set.
    echo Set OPENSSL_ROOT_DIR to the OpenSSL installation containing include and lib directories.
    exit /b 1
)

if not exist "%OPENSSL_ROOT_DIR%\include\openssl\ssl.h" (
    echo ERROR: OpenSSL headers were not found under OPENSSL_ROOT_DIR.
    exit /b 1
)

if not exist build\windows mkdir build\windows

cl /nologo /std:c17 /W4 /TC /Iinclude /I"%OPENSSL_ROOT_DIR%\include" /Fo"build\windows\\" /Fe"build\windows\rictus.exe" src\main.c src\rictus.c src\config.c src\tls_openssl.c platforms\windows\rictus_net_windows.c /link /LIBPATH:"%OPENSSL_ROOT_DIR%\lib" libssl.lib libcrypto.lib ws2_32.lib crypt32.lib
if errorlevel 1 exit /b 1

echo BUILD SUCCESSFUL
exit /b 0
