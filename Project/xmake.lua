-- 设置项目名称
set_project("MyProject")

-- 添加调试和发布模式
add_rules("mode.debug", "mode.release")

target("main.out")
    -- 设置为二进制可执行程序
    set_kind("binary")
    
    -- 设置 C++ 标准 (xmake 会自动转换成 MSVC 的 /std:c++14 等格式)
    set_languages("c++14")
    
    -- 指定源文件路径
    add_files("sources/*.cpp")
    
    -- 指定头文件搜索路径
    add_includedirs("includefile")

    -- 设置输出路径为根目录
    set_targetdir("./")
    
    -- === 平台特定配置 ===
    if is_plat("windows") then
        -- Windows 下链接 socket 库
        add_syslinks("ws2_32")
        -- 解决中文注释乱码报错 (C4819)，强制使用 UTF-8
        add_cxflags("/utf-8")
        -- 屏蔽一些不安全的 CRT 警告 (如 strcpy unsafe)
        add_defines("_CRT_SECURE_NO_WARNINGS")
    elseif is_plat("linux", "macosx") then
        -- Linux 下链接 pthread
        add_syslinks("pthread")
    end