@echo off
chcp 65001 >nul
setlocal

:: ==========================================
:: 核心修复：锁定工作目录
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

:: 【修复点】去掉了 echo 中的圆括号，防止语法解析错误
if exist packages (
    echo [INFO] Installing from local packages - Offline Mode...
    pip install --no-index --find-links=./packages -r requirements.txt
) else (
    echo [WARN] Packages folder missing. Installing online...
    pip install -r requirements.txt
)

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