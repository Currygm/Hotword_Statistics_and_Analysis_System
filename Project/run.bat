@echo off
chcp 65001 >nul
setlocal

:: ==========================================
:: 切换到脚本所在目录
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

if not exist venv (
    echo [INFO] Creating venv...
    python -m venv venv
) else (
    echo [INFO] venv already exists.
)

call venv\Scripts\activate

echo.
echo ===================================================
echo [INFO] Step 3: Installing Dependencies...
echo ===================================================

:: --- 【修改开始】 版本检测与安装逻辑 ---

:: 1. 获取当前 venv 中的 Python 版本 (例如 3.11)
for /f "delims=" %%v in ('python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')"') do set CURRENT_VER=%%v

echo [INFO] Current Python version in venv: %CURRENT_VER%
echo [INFO] Target offline package version: 3.11

:: 2. 设置安装模式，默认为 ONLINE
set INSTALL_MODE=ONLINE

:: 3. 如果 packages 存在 且 版本是 3.11，则切换为 OFFLINE
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
    :: 打印具体原因
    if "%CURRENT_VER%" neq "3.11" echo [WARN] Python version mismatch! Local packages require 3.11.
    if not exist packages echo [WARN] 'packages' folder missing.
    
    echo [INFO] Switching to online installation using Tsinghua Mirror...
    pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple
)

:: --- 【修改结束】 ---

if %errorlevel% neq 0 (
    echo [ERROR] Failed to install dependencies.
    echo [HINT] Try running as standard user, not Administrator.
    pause
    exit /b 1
)

echo.
echo ===================================================
echo [INFO] Step 4: Compiling C++ Project...
echo ===================================================

where xmake >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] xmake not found.
    echo [HINT] Please install xmake or add it to PATH.
    pause
    exit /b 1
)

call xmake clean >nul 2>&1
call xmake
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b 1
)

echo.
echo ===================================================
echo [INFO] Step 5: Launching System...
echo ===================================================

if exist sources\app.py (
    echo [INFO] Starting Streamlit...
    echo [INFO] The browser should open automatically.
    streamlit run sources/app.py
) else (
    echo [ERROR] sources/app.py not found!
    pause
    exit /b 1
)

pause