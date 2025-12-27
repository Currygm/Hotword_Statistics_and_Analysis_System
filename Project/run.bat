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
    echo [ERROR] Python not found!
    echo [HINT] Please install Python and add it to PATH.
    pause
    exit /b 1
)

echo.
echo ===================================================
echo [INFO] Step 2: Setting up Virtual Environment...
echo ===================================================

:: 修改点 1：如果存在 venv 则强行删除，实现“覆盖”
if exist venv (
    echo [INFO] Old venv detected. Removing to perform a fresh install...
    rd /s /q venv
)

echo [INFO] Creating new venv...
python -m venv venv

call venv\Scripts\activate

echo.
echo ===================================================
echo [INFO] Step 3: Installing Dependencies...
echo ===================================================

:: 1. 获取当前 venv 中的 Python 版本 (例如 3.11)
for /f "delims=" %%v in ('python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')"') do set CURRENT_VER=%%v

echo [INFO] Current Python version in venv: %CURRENT_VER%
echo [INFO] Target offline package version: 3.11

:: 2. 设置安装模式，默认为 ONLINE
set INSTALL_MODE=ONLINE

:: 3. 如果 packages 存在 且 版本是 3.11，切换为 OFFLINE
if exist packages (
    if "%CURRENT_VER%"=="3.11" (
        set INSTALL_MODE=OFFLINE
    )
)

:: 4. 根据模式执行安装
if "%INSTALL_MODE%"=="OFFLINE" (
    echo [INFO] Version matches 3.11. Installing from local packages - Offline Mode...
    pip install --no-index --find-links=./packages -r requirements.txt
) else (
    if "%CURRENT_VER%" neq "3.11" echo [WARN] Python version mismatch! Local packages require 3.11.
    if not exist packages echo [WARN] 'packages' folder missing.
    
    echo [INFO] Switching to online installation using Tsinghua Mirror...
    pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple
)

if %errorlevel% neq 0 (
    echo [ERROR] Failed to install dependencies.
    pause
    exit /b 1
)

echo.
echo ===================================================
echo [INFO] Step 4: Checking & Installing xmake...
echo ===================================================

:: 修改点 2：自动安装 xmake 逻辑
where xmake >nul 2>&1
if %errorlevel% neq 0 (
    echo [INFO] xmake not found. Starting automatic installation...
    :: 调用官方 PowerShell 安装脚本
    powershell -Command "Invoke-Expression (Invoke-RestMethod https://xmake.io/install.ps1)"
    
    :: 将安装路径加入当前会话的 PATH (默认路径通常在 %USERPROFILE%\.xmake\bin)
    set "PATH=%PATH%;%USERPROFILE%\.xmake\bin"
    
    :: 再次验证安装
    where xmake >nul 2>&1
    if %errorlevel% neq 0 (
        echo [ERROR] Automatic xmake installation failed. Please install it manually from https://xmake.io
        pause
        exit /b 1
    )
    echo [INFO] xmake installed successfully.
) else (
    echo [INFO] xmake is already installed.
)

echo.
echo ===================================================
echo [INFO] Step 5: Compiling C++ Project...
echo ===================================================

call xmake clean >nul 2>&1
call xmake -y
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b 1
)

echo.
echo ===================================================
echo [INFO] Step 6: Launching System...
echo ===================================================

if exist sources\app.py (
    echo [INFO] Starting Streamlit...
    streamlit run sources/app.py
) else (
    echo [ERROR] sources/app.py not found!
    pause
    exit /b 1
)

pause