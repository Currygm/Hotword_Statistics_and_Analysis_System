# 热词统计与分析系统

## 环境依赖
1. C++ 编译器 (GCC/Clang/MSVC)
2. xmake 构建工具
3. Python 3.8+

## 如何运行 (一条指令)

### Windows 用户:
直接双击运行 `run.bat`，或在命令行执行:
> .\run.bat

### Linux / macOS 用户:
在终端执行:
$ bash run.sh

## 脚本说明
脚本会自动完成以下步骤：
1. 创建 Python 虚拟环境 (venv)
2. 安装必要库 (streamlit, pandas, altair)
3. 使用 xmake 编译 C++ 核心
4. 启动 Web 界面

# 删除编译产物
xmake clean
rm -f main.out main.exe

# 删除虚拟环境 (这是别人的电脑必须自己生成的)
rm -rf venv

# 删除临时数据
rm -rf temp
rm -rf .xmake
rm -rf build
rm -rf __pycache__