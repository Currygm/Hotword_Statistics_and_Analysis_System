import streamlit as st
import socket
import json
import pandas as pd
import subprocess
import os
import altair as alt  # 用于高级绘图

# --- 页面设置 ---
st.set_page_config(layout="wide", page_title="热词分析系统 (WSL版)")

st.title("🔥 C++ 热词统计与分析系统")

# --- 侧边栏：控制面板 ---
st.sidebar.header("1. 数据与参数配置")

# 1. 文件上传
uploaded_file = st.sidebar.file_uploader("上传 input.txt", type=["txt"])
input_filename = "temp_input.txt" # 默认临时文件名，用于保存上传的数据供C++读取

# 2. 算法参数
stride_val = st.sidebar.number_input("滑动步长 (Stride)", min_value=1, value=120)
# 这个 K 是前端显示的 K，不影响 output.txt (output.txt 由文件内 ACTION 决定)
gui_k_value = st.sidebar.slider("前端显示 Top-K", 1, 50, 20) 

# 3. 输出文件名设置
output_filename = st.sidebar.text_input("指定输出文件名", "my_output.txt")

# --- 核心处理函数 ---
def run_analysis():
    # 1. 保存上传的文件到本地 (供 C++ 读取)
    if uploaded_file is not None:
        with open(input_filename, "wb") as f:
            f.write(uploaded_file.getbuffer())
    else:
        st.error("请先上传文件！")
        return

    # 初始化状态
    st.session_state.history = []     # 存储所有时间点数据
    st.session_state.timestamps = []  # 存储时间轴
    st.session_state.processing_done = False
    
    # 2. 启动 UDP 监听
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # 允许地址复用，防止快速重启时端口被占用
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        sock.bind(('0.0.0.0', 9999))
    except OSError:
        st.error("端口 9999 被占用，请稍后重试或关闭其他正在运行的程序。")
        return

    sock.settimeout(5.0) # 设置超时，防止无限等待

    # 3. 构建 C++ 运行命令
    # 假设你的可执行文件叫 ./main.out，请确保它在当前目录下
    if not os.path.exists("./main.out"):
        st.error("找不到 ./main.out，请先编译 C++ 代码！(执行: g++ main.cpp -o main.out)")
        return

    cmd = [
        "./main.out", 
        "-i", input_filename,       # 输入文件
        "-o", output_filename,      # 输出文件
        "-s", str(stride_val)       # 步长
    ]
    
    status_text = st.empty()
    
    # 4. 启动子进程
    try:
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except Exception as e:
        st.error(f"启动 C++ 失败: {e}")
        return

    # 5. 循环接收数据
    counter = 0
    while True:
        try:
            data, _ = sock.recvfrom(65536) # 大缓冲区
            msg = data.decode('utf-8')
            
            if msg == "EOF":
                break
            if "error" in msg:
                st.error(f"后端错误: {msg}")
                break
                
            # 解析 JSON
            record = json.loads(msg) 
            # record 格式: {'time': '[00:00:00]', 'top_k': [...]}
            
            st.session_state.history.append(record)
            st.session_state.timestamps.append(record['time'])
            
            counter += 1
            if counter % 50 == 0:
                status_text.text(f"正在处理... 已分析 {counter} 个时间窗口")
                
        except socket.timeout:
            # 检查进程是否已经退出
            if process.poll() is not None: 
                break
            continue
        except Exception as e:
            st.error(f"数据接收错误: {e}")
            break

    sock.close()
    process.wait() # 等待 C++ 完全写入文件
    
    st.session_state.processing_done = True
    status_text.success(f"分析完成！共处理 {len(st.session_state.history)} 个时间点。")
    st.rerun()

# --- 按钮区域 ---
if st.sidebar.button("🚀 开始分析"):
    with st.spinner("C++ 后端正在计算..."):
        run_analysis()

# --- 结果展示区域 ---
if 'processing_done' in st.session_state and st.session_state.processing_done:
    
    st.markdown("---")
    
    # === 左半部分：下载 output.txt ===
    col_dl, col_vis = st.columns([1, 3])
    
    with col_dl:
        st.subheader("📂 结果下载")
        st.info("output.txt 包含基于文件中 [ACTION] 指令生成的结果。")
        
        # 智能路径查找逻辑
        possible_paths = [
            output_filename,                       # 当前目录
            os.path.join("data", output_filename), # data 目录
            "data/" + output_filename              # 硬编码 data 目录
        ]
        
        found_path = None
        for p in possible_paths:
            if os.path.exists(p):
                found_path = p
                break
        
        if found_path:
            with open(found_path, "r", encoding="utf-8") as f:
                st.download_button(
                    label=f"📥 下载 {output_filename}",
                    data=f,
                    file_name=output_filename,
                    mime="text/plain"
                )
        else:
            st.warning(f"未找到输出文件 {output_filename}，可能 C++ 运行出错或权限不足。")

    # === 右半部分：交互式可视化 ===
    with col_vis:
        st.subheader("📊 交互式热词回放")
        if len(st.session_state.timestamps) > 0:
            # 1. 时间轴滑块
            selected_time = st.select_slider(
                "拖动滑块查看任意时刻的词频状态：",
                options=st.session_state.timestamps
            )
            
            # 2. 查找对应数据
            idx = st.session_state.timestamps.index(selected_time)
            data_point = st.session_state.history[idx]
            
            # 3. 截取 Top-K (根据前端设置的 K 值)
            full_list = data_point['top_k']
            display_list = full_list[:gui_k_value] # Python 切片
            
            # 4. 高级绘图 (Altair)
            if display_list:
                df = pd.DataFrame(display_list)
                
                # 按数量降序排序，确保柱状图左高右低
                df = df.sort_values(by='count', ascending=False)
                
                # 使用 Altair 绘图
                chart = alt.Chart(df).mark_bar().encode(
                    x=alt.X('word', 
                            sort='-y',       # 强制按照 Y 轴数值倒序排列
                            axis=None        # 隐藏 X 轴标签（不显示词）
                    ),
                    y=alt.Y('count', title='出现频次'), # Y 轴标题
                    tooltip=['word', 'count'],         # 鼠标悬停显示详情
                    color=alt.value("#36A2EB")         # 统一柱状图颜色
                ).properties(
                    height=450 # 图表高度
                )
                
                st.altair_chart(chart, use_container_width=True)
            else:
                st.write("当前时刻无热词数据。")
        else:
            st.warning("无数据可展示。")
else:
    st.info("请在左侧上传文件并点击【开始分析】。")