import streamlit as st
import socket
import json
import pandas as pd
import subprocess
import os
import platform  # <--- 新增模块
import altair as alt

# --- 0. 基础配置与目录初始化 ---
st.set_page_config(layout="wide", page_title="高级热词分析系统")

TEMP_DIR = "temp"
if not os.path.exists(TEMP_DIR):
    os.makedirs(TEMP_DIR)

st.title("🔥 高级热词统计与分析系统")

# --- 侧边栏 ---
st.sidebar.header("1. 数据源")
uploaded_file = st.sidebar.file_uploader("上传 input.txt", type=["txt"])

# 【修改点1】将临时输入文件路径指向 temp 文件夹
input_filename = os.path.join(TEMP_DIR, "temp_input.txt")

st.sidebar.header("2. 核心参数")
stride_val = st.sidebar.number_input("滑动步长 (Stride)", min_value=1, value=120)

# 前端自定义K的最大值
max_k_limit = st.sidebar.number_input("前端最大 Top-K 限制", min_value=10, max_value=500, value=50)
gui_k_value = st.sidebar.slider("当前显示 Top-K", 1, max_k_limit, min(20, max_k_limit))

# 分词模式选择
st.sidebar.header("3. 分词模式")
seg_mode = st.sidebar.selectbox(
    "选择分词算法",
    ("Cut(HMM)", "Cut(NoHMM)", "CutForSearch"),
    index=0,
    help="HMM: 新词识别能力强; NoHMM: 速度快但对新词弱; Search: 适合搜索引擎，分词更细"
)

# 自定义词典
with st.sidebar.expander("4. 自定义词典配置", expanded=False):
    st.markdown("**用户专用词 (User Dict)**")
    st.caption("强制保留的词，一行一个，或空格分隔")
    user_dict_input = st.text_area("输入专用词", height=150, placeholder="在此输入内容")
    
    st.markdown("**自定义停用词 (Stop Words)**")
    st.caption("强制过滤的词/敏感词，一行一个")
    stop_word_input = st.text_area("输入停用词", height=150, placeholder="在此输入内容")

output_filename = st.sidebar.text_input("指定输出文件名", "my_output.txt")

# --- 核心逻辑 ---
def run_analysis():
    # 1. 保存上传数据
    if uploaded_file is not None:
        with open(input_filename, "wb") as f:
            f.write(uploaded_file.getbuffer())
    else:
        st.error("请先上传文件！")
        return

    # 2. 将字典临时文件放入 temp 文件夹
    temp_user_dict = os.path.join(TEMP_DIR, "temp_user.dict")
    temp_stop_words = os.path.join(TEMP_DIR, "temp_stop.txt")
    
    with open(temp_user_dict, "w", encoding="utf-8") as f:
        content = user_dict_input.replace(" ", "\n")
        f.write(content)
        
    with open(temp_stop_words, "w", encoding="utf-8") as f:
        content = stop_word_input.replace(" ", "\n")
        f.write(content)

    # 3. 准备 Socket
    st.session_state.history = []
    st.session_state.timestamps = []
    st.session_state.processing_done = False
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        sock.bind(('0.0.0.0', 9999))
    except OSError:
        st.error("端口 9999 被占用，请稍后再试。")
        return
    sock.settimeout(5.0)

    # 4. 构建命令 (【核心修复】：智能识别操作系统)
    # ---------------------------------------------------------
    system_type = platform.system() # 获取操作系统类型
    
    executable_name = "main.out" # 默认 Linux/Mac
    
    if system_type == "Windows":
        # Windows 下 xmake 默认会生成 .exe 后缀
        # 我们优先找 main.exe
        if os.path.exists("main.exe"):
            executable_name = "main.exe"
        elif os.path.exists("main.out.exe"):
             executable_name = "main.out.exe"
        # 如果只有 main.out 且是 Windows，这通常是问题所在，但我们先试着指向它
        elif os.path.exists("main.out"):
             executable_name = "main.out" 
    
    # 检查文件是否存在
    if not os.path.exists(executable_name):
        st.error(f"找不到可执行文件: {executable_name}")
        st.info(f"当前目录文件列表: {os.listdir('.')}")
        return

    # 在 Windows 上调用 subprocess 时，最好不要带 './' 前缀，直接用文件名
    cmd_exe = executable_name if system_type == "Windows" else f"./{executable_name}"

    cmd = [
        cmd_exe,
        "-i", input_filename,
        "-o", output_filename,
        "-s", str(stride_val),
        "-m", seg_mode,
        "-u", temp_user_dict,
        "-w", temp_stop_words,
        "-k_limit", str(max_k_limit)
    ]
    # ---------------------------------------------------------
    
    status_text = st.empty()
    
    try:
        # Windows 下 subprocess.Popen 不需要 shell=True (通常)
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except Exception as e:
        st.error(f"启动失败: {e}")
        st.error(f"尝试执行的命令: {cmd}")
        return

    # 5. 接收循环 (保持不变)
    counter = 0
    while True:
        try:
            data, _ = sock.recvfrom(65536 * 2) 
            msg = data.decode('utf-8')
            if msg == "EOF": break
            if "error" in msg:
                st.error(f"后端错误: {msg}")
                break
            
            record = json.loads(msg)
            st.session_state.history.append(record)
            st.session_state.timestamps.append(record['time'])
            
            counter += 1
            if counter % 50 == 0:
                status_text.text(f"处理中... 窗口 {counter}")
        except socket.timeout:
            # 检查子进程是否挂了
            return_code = process.poll()
            if return_code is not None:
                # 进程结束了，看看是不是报错了
                stderr_output = process.stderr.read().decode('gbk', errors='ignore') # Windows下可能是GBK
                if return_code != 0:
                    st.error(f"C++ 程序异常退出，错误码: {return_code}")
                    if stderr_output:
                        st.code(stderr_output)
                break
            continue
        except Exception as e:
            st.error(f"发生未知错误: {e}")
            break

    sock.close()
    process.wait()
    st.session_state.processing_done = True
    status_text.success(f"完成！共处理 {len(st.session_state.history)} 个时间点")
    st.rerun()

if st.sidebar.button("🚀 开始高级分析"):
    with st.spinner("正在初始化分词引擎并计算..."):
        run_analysis()

# --- 结果展示 (修改为上下布局) ---
if 'processing_done' in st.session_state and st.session_state.processing_done:
    st.markdown("---")
    
    # 【修改点3】去掉了 st.columns，直接按顺序书写
    
    # 1. 上方：交互式分析图表
    st.subheader("📊 交互式分析")
    
    if len(st.session_state.timestamps) > 0:
        # 时间轴滑块
        selected_time = st.select_slider("时间轴", options=st.session_state.timestamps)
        idx = st.session_state.timestamps.index(selected_time)
        data_point = st.session_state.history[idx]
        
        # 使用用户设置的 gui_k_value 进行切片
        full_list = data_point['top_k']
        display_list = full_list[:gui_k_value]
        
        if display_list:
            df = pd.DataFrame(display_list)
            # 排序
            df = df.sort_values(by='count', ascending=False)
            
            # Altair 绘图 (隐藏 X 轴文字，按 Y 轴排序)
            chart = alt.Chart(df).mark_bar().encode(
                x=alt.X('word', sort='-y', axis=None),
                y=alt.Y('count', title='频次'),
                tooltip=['word', 'count'],
                color=alt.value("#36A2EB")
            ).properties(height=450) # 可以适当增加高度
            
            st.altair_chart(chart, use_container_width=True)
        else:
            st.write("该时间点无 Top-K 数据")
    else:
        st.warning("无数据记录")

    st.markdown("---") # 添加一条分割线

    # 2. 下方：文件下载
    st.subheader("📂 结果下载")
    
    # 智能查找路径逻辑
    possible_paths = [
        output_filename,                       # 当前目录
        os.path.join("data", output_filename), # data 目录
        "data/" + output_filename              
    ]
    
    found_path = None
    for p in possible_paths:
        if os.path.exists(p):
            found_path = p
            break
    
    if found_path:
        with open(found_path, "r", encoding="utf-8") as f:
            st.download_button(
                label=f"📥 下载完整结果 ({output_filename})",
                data=f,
                file_name=output_filename,
                mime="text/plain"
            )
        st.info("提示：output.txt 包含基于 input.txt 中 [ACTION] 指令生成的详细报告。")
    else:
        st.warning(f"未找到输出文件 {output_filename}。")

elif 'processing_done' not in st.session_state:
    st.info("👋 请在左侧配置参数并上传文件，然后点击【开始高级分析】")