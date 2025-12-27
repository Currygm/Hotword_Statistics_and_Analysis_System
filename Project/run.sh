#!/bin/bash
set -e

echo "==================================================="
echo "[INFO] Step 1: Checking Python Environment"
echo "==================================================="

# 优先检查 python3
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
# 修改点：如果存在 venv 则删除，实现“覆盖”效果
if [ -d "venv" ]; then
    echo "[INFO] Old venv detected. Removing to perform a fresh install..."
    rm -rf venv
fi

echo "[INFO] Creating new venv..."
$PYTHON_CMD -m venv venv

# 激活环境
source venv/bin/activate

echo ""
echo "==================================================="
echo "[INFO] Step 3: Installing Dependencies"
echo "==================================================="

CURRENT_VER=$(python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")

echo "[INFO] Current Python version in venv: $CURRENT_VER"
echo "[INFO] Target offline package version: 3.11"

if [[ "$CURRENT_VER" == "3.11" ]] && [ -d "packages" ]; then
    echo "[INFO] Version matches 3.11. Installing from local packages (Offline)..."
    pip install --no-index --find-links=./packages -r requirements.txt
else
    if [[ "$CURRENT_VER" != "3.11" ]]; then
        echo "[WARN] Python version mismatch! Local packages require 3.11."
    else
        echo "[WARN] 'packages' directory missing."
    fi
    
    echo "[INFO] Switching to online installation using Tsinghua Mirror..."
    pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple
fi

echo ""
echo "==================================================="
echo "[INFO] Step 4: Checking & Installing xmake"
echo "==================================================="
# 修改点：自动安装 xmake 逻辑
if ! command -v xmake &>/dev/null; then
    echo "[INFO] xmake not found. Starting automatic installation..."
    # 检查是否有 curl
    if command -v curl &>/dev/null; then
        curl -fsSL https://xmake.io/shget.lua | bash
        # 安装后通常需要将路径加入当前会话
        export PATH=$PATH:$HOME/.local/bin
        # 尝试加载 xmake 环境变量配置文件
        [ -f ~/.xmake/profile ] && source ~/.xmake/profile || true
    else
        echo "[ERROR] 'curl' is required to install xmake. Please install curl first."
        # 如果没有 curl，尝试跳过安装使用下面的 g++ 备选方案
    fi
else
    echo "[INFO] xmake is already installed."
fi

echo ""
echo "==================================================="
echo "[INFO] Step 5: Compiling C++ Project"
echo "==================================================="
if command -v xmake &>/dev/null; then
    echo "[INFO] Compiling with xmake..."
    xmake -y # -y 自动确认
else
    echo "[WARN] xmake still not available. Using g++ fallback..."
    g++ sources/*.cpp -o main.out -Iincludefile -lpthread -std=c++11 -O3
fi

echo ""
echo "==================================================="
echo "[INFO] Step 6: Launching Streamlit App"
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