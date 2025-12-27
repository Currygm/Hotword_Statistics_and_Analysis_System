#!/bin/bash
# 注意：set -e 会导致任何指令失败时立即退出，为了允许失败后切换逻辑，
# 我们在可能失败的步骤后做了特殊处理。
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
# 如果存在 venv 则删除，实现“覆盖”效果
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

# --- 逻辑 A: 原有的 apt 安装逻辑 (不做变动) ---
if ! command -v xmake &>/dev/null; then
    echo "[INFO] xmake not found. Starting automatic installation via apt..."
    
    if command -v apt-get &>/dev/null; then
        # 这里用 ( ) 包裹并配合 || true 是为了防止 set -e 在 apt 失败时直接终止脚本，
        # 从而让我们有机会进入下面的逻辑 B。
        (
            # 1. 添加 PPA
            if command -v add-apt-repository &>/dev/null; then
                echo "[INFO] Adding xmake PPA..."
                sudo add-apt-repository ppa:xmake-io/xmake -y
            fi
            
            echo "[INFO] Updating apt-get..."
            sudo apt-get update
            
            # 2. 先尝试删除可能导致冲突的系统自带旧版包
            echo "[INFO] Removing potential conflicting old packages (xmake-data)..."
            sudo apt-get remove xmake-data xmake -y >/dev/null 2>&1 || true
            
            # 3. 安装新版
            echo "[INFO] Installing latest xmake via apt..."
            sudo apt-get install xmake -y
        ) || echo "[WARN] apt-get installation process encountered errors."
    else
        echo "[WARN] 'apt' package manager not found. Skipping apt logic."
    fi
fi

# --- 逻辑 B: 新增的备选方案 (仅当上述逻辑 A 失败或跳过后执行) ---
if ! command -v xmake &>/dev/null; then
    echo "[INFO] xmake still not found. Falling back to official web installer..."
    if command -v curl &>/dev/null; then
        curl -fsSL https://xmake.io/shget.lua | bash
        # 官网脚本通常安装在 ~/.local/bin，我们需要更新当前会话的路径
        export PATH=$PATH:$HOME/.local/bin
        # 尝试加载 xmake 生成的 profile
        [ -f ~/.xmake/profile ] && source ~/.xmake/profile || true
    elif command -v wget &>/dev/null; then
        wget https://xmake.io/shget.lua -O shget.lua
        bash shget.lua
        export PATH=$PATH:$HOME/.local/bin
        [ -f ~/.xmake/profile ] && source ~/.xmake/profile || true
        rm shget.lua
    else
        echo "[ERROR] Neither 'apt', 'curl' nor 'wget' could install xmake."
        echo "[HINT] Please install xmake manually: https://xmake.io"
        # 不退出，尝试下一步的 g++ 兜底
    fi
fi

# 最终检测状态显示
if command -v xmake &>/dev/null; then
    echo "[INFO] xmake is ready: $(xmake --version | head -n 1)"
else
    echo "[WARN] xmake is not available in PATH."
fi

echo ""
echo "==================================================="
echo "[INFO] Step 5: Compiling C++ Project"
echo "==================================================="
if command -v xmake &>/dev/null; then
    echo "[INFO] Compiling with xmake..."
    xmake f -y
    xmake -y
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