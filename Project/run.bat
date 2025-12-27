@echo off
chcp 65001 >nul
setlocal

:: ==========================================
:: 切换至脚本所在目录
:: ==========================================
cd /d "%~dp0"

echo ===================================================
echo [INFO] Step 1: Checking Python Environment...
echo ===================================================

python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Python not found.
    echo [HINT] Please install Python and add it to PATH.
    pause
    exit /b 1
)

echo.
echo ===================================================
echo [INFO] Step 2: Setting up Virtual Environment...
echo ===================================================

if exist venv (
    echo [INFO] Old venv detected. Removing for a fresh install...
    rd /s /q venv
)

echo [INFO] Creating new venv...
python -m venv venv

:: 激活虚拟环境
call venv\Scripts\activate

echo.
echo ===================================================
echo [INFO] Step 3: Installing Dependencies...
echo ===================================================

:: 获取 Python 版本 (例如 3.11)
for /f "delims=" %%v in ('python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')"') do set CURRENT_VER=%%v

echo [INFO] Current Python version in venv: %CURRENT_VER%
echo [INFO] Target offline package version: 3.11

set "MODE=ONLINE"
if "%CURRENT_VER%"=="3.11" (
    if exist packages (
        set "MODE=OFFLINE"
    )
)

if "%MODE%"=="OFFLINE" (
    echo [INFO] Installing from local packages - Offline Mode
    pip install --no-index --find-links=./packages -r requirements.txt
) else (
    echo [INFO] Version mismatch or packages folder missing.
    echo [INFO] Switching to online installation - Tsinghua Mirror
    pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple
)

if %errorlevel% neq 0 (
    echo [ERROR] Failed to install dependencies.
    pause
    exit /b 1
)

echo.
echo ===================================================
echo [INFO] Step 4: Checking and Installing xmake...
echo ===================================================

where xmake >nul 2>&1
if %errorlevel% equ 0 goto :XMAKE_READY

echo [INFO] xmake not found. Starting automatic installation...

:: 1. 尝试 winget
where winget >nul 2>&1
if %errorlevel% equ 0 (
    echo [INFO] Attempting installation via winget...
    winget install xmake-io.xmake --silent --accept-package-agreements --accept-source-agreements
)

:: 2. 如果依然没有，试 PowerShell
where xmake >nul 2>&1
if %errorlevel% neq 0 (
    echo [INFO] Falling back to official PowerShell installer...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-Expression (Invoke-RestMethod https://xmake.io/install.ps1)"
)

:: 注入安装路径
set "PATH=%PATH%;%ProgramFiles%\xmake;%LOCALAPPDATA%\xmake;%USERPROFILE%\.xmake\bin"

:XMAKE_READY
where xmake >nul 2>&1
if %errorlevel% equ 0 (
    echo [INFO] xmake is ready.
) else (
    echo [WARN] All xmake installation methods failed.
)

echo.
echo ===================================================
echo [INFO] Step 5: Compiling C++ Project...
echo ===================================================

:: 优先检查 xmake
where xmake >nul 2>&1
if %errorlevel% neq 0 goto :TRY_GPP

echo [INFO] Using xmake to compile...
call xmake f -y
call xmake clean >nul 2>&1
call xmake -y
if %errorlevel% equ 0 goto :COMPILE_SUCCESS
echo [ERROR] xmake compilation failed.
pause
exit /b 1

:TRY_GPP
:: 检查 g++
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Neither xmake nor g++ found.
    echo [HINT] Please install a C++ compiler or xmake.
    pause
    exit /b 1
)

echo [WARN] xmake not found. Using g++ fallback...
g++ sources\*.cpp -o main.out -Iincludefile -std=c++14 -O3 -lws2_32 -D_CRT_SECURE_NO_WARNINGS
if %errorlevel% neq 0 (
    echo [ERROR] g++ compilation failed.
    pause
    exit /b 1
)

:COMPILE_SUCCESS
echo [INFO] Compilation successful.

echo.
echo ===================================================
echo [INFO] Step 6: Launching System...
echo ===================================================

if exist sources\app.py (
    echo [INFO] Starting Streamlit...
    streamlit run sources/app.py
) else (
    echo [ERROR] sources/app.py not found.
    pause
    exit /b 1
)

pause