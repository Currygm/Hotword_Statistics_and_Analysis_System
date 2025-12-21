-- 设置项目名称
set_project("MyProject")

-- 添加调试和发布模式
add_rules("mode.debug", "mode.release")

target("main.out")
    -- 设置为二进制可执行程序
    set_kind("binary")
    
    -- 设置 C++ 标准
    set_languages("c++11")
    
    -- 1. 指定源文件路径：添加 sources 文件夹下所有的 .cpp 文件
    add_files("sources/*.cpp")
    
    -- 2. 指定头文件搜索路径：
    -- 对应之前的 -I./cppjieba 和新存放 config.h 的路径
    add_includedirs("includefile")

    -- 设置输出文件名及路径（生成在项目根目录）
    set_targetdir("./")
    
    -- 3. 添加线程库
    add_syslinks("pthread")
