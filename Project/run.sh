#!/bin/bash
set -e

echo "==================================================="
echo "[INFO] Step 1: Checking Python Environment"
echo "==================================================="

if command -v python3 &>/dev/null; then
    PYTHON_CMD=python3
elif command -v python &>/dev/null; then
    PYTHON_CMD=python
else
    echo "[ERROR] Python is not installed."
    exit 1
fi

echo ""
echo "==================================================="
echo "[INFO] Step 2: Setting up Virtual Environment"
echo "==================================================="
if [ ! -d "venv" ]; then
    echo "[INFO] Creating venv..."
    $PYTHON_CMD -m venv venv
else
    echo "[INFO] venv already exists."
fi

# 激活环境
source venv/bin/activate

echo ""
echo "==================================================="
echo "[INFO] Step 3: Installing Dependencies (Offline)"
echo "==================================================="

# 【修改点】现在 Linux 也强制使用 packages 文件夹进行安装
if [ -d "packages" ]; then
    echo "[INFO] Installing from local packages..."
    pip install --no-index --find-links=./packages -r requirements.txt
else
    echo "[WARN] 'packages' folder missing. Falling back to online install..."
    pip install -r requirements.txt
fi

echo ""
echo "==================================================="
echo "[INFO] Step 4: Compiling C++ Project"
echo "==================================================="
if command -v xmake &>/dev/null; then
    xmake
else
    echo "[WARN] xmake not found. Using g++ fallback..."
    # 确保 dataDir 路径逻辑正确
    g++ sources/*.cpp -o main.out -Iincludefile -lpthread -std=c++11
fi

echo ""
echo "==================================================="
echo "[INFO] Step 5: Launching Streamlit App"
echo "==================================================="

if [ -f "main.out" ]; then
    chmod +x main.out
fi

if [ -f "sources/app.py" ]; then
    streamlit run sources/app.py
else
    echo "[ERROR] sources/app.py not found!"
    exit 1
fi