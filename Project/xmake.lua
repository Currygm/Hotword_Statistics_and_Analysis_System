-- 设置项目名称
set_project("MyProject")

-- 添加调试和发布模式规则
add_rules("mode.debug", "mode.release")

target("main.out")
    -- 设置为二进制可执行程序
    set_kind("binary")
    
    -- 设置 C++ 标准
    -- 建议使用 c++11 或更高 (c++17 推荐，但在 WSL 上 c++11 也完全支持你的代码)
    set_languages("c++11")
    
    -- 1. 指定源文件路径
    -- 添加 sources 文件夹下所有的 .cpp 文件
    add_files("sources/*.cpp")
    
    -- 2. 指定头文件搜索路径
    -- 这样你代码里的 #include "config.h" 或 jieba 的头文件能被正确找到
    add_includedirs("includefile")

    -- 设置输出文件名及路径（生成在项目根目录，方便 python 调用）
    set_targetdir("./")
    
    -- 3. 系统库链接与编译选项
    if is_plat("linux", "macosx") then
        -- Linux/WSL/macOS 下：
        -- 1. Socket 是 glibc 的一部分，不需要额外链接库
        -- 2. std::thread 需要 pthread 库
        add_syslinks("pthread")
        
    elseif is_plat("windows", "mingw") then
        -- 如果要在 Windows 原生环境编译：
        -- 需要链接 Winsock2 库 (即 ws2_32) 才能使用 socket
        add_syslinks("ws2_32") 
    end
    
    -- (可选) 优化：如果使用 cppjieba 产生大量警告，可以屏蔽
    -- add_cxflags("-w")