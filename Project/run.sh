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
echo "[INFO] Step 3: Installing Dependencies"
echo "==================================================="

# --- 【修改开始】 版本检测与安装逻辑 ---

# 获取当前虚拟环境的 Python 主版本.次版本 (例如 3.11)
# 使用 python -c 确保获取的是激活后的环境版本
CURRENT_VER=$(python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")

echo "[INFO] Current Python version in venv: $CURRENT_VER"
echo "[INFO] Target offline package version: 3.11"

# 判断逻辑：
# 1. 如果版本是 3.11 且 packages 目录存在 -> 离线安装
# 2. 否则 (版本不对 或 目录不存在) -> 在线安装 (清华源)
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
# --- 【修改结束】 ---

echo ""
echo "==================================================="
echo "[INFO] Step 4: Compiling C++ Project"
echo "==================================================="
if command -v xmake &>/dev/null; then
    xmake
else
    echo "[WARN] xmake not found. Using g++ fallback..."
    # 增加 -O3 优化以提升性能
    g++ sources/*.cpp -o main.out -Iincludefile -lpthread -std=c++11 -O3
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