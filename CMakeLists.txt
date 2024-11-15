cmake_minimum_required(VERSION 3.10)

project(AutoTrayIt VERSION 0.1.0) # 项目名称和版本号

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/tomlplusplus)

# 编译选项
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-finput-charset=UTF-8>")
add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-fexec-charset=UTF-8>")
add_compile_options("$<$<CXX_COMPILER_ID:Clang>:-finput-charset=UTF-8>")
add_compile_options("$<$<CXX_COMPILER_ID:Clang>:-fexec-charset=UTF-8>")

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/inc)

# 源文件
file(GLOB SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/*.c)

# 可执行文件
add_executable(AutoTrayIt WIN32 ${SOURCE_FILES})

# 定义 PROJECT_VERSION 预处理器宏
target_compile_definitions(AutoTrayIt PRIVATE PROJECT_VERSION=${PROJECT_VERSION})

# 链接库
target_link_libraries(AutoTrayIt PRIVATE user32 gdi32 shell32 tomlplusplus::tomlplusplus)


# 设置架构名称后缀
if (CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
    set(ARCH_SUFFIX "x86")
elseif (CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
    set(ARCH_SUFFIX "x64")
else ()
    set(ARCH_SUFFIX "")
endif ()


# 设置输出名称并添加架构后缀
set_target_properties(AutoTrayIt PROPERTIES
        OUTPUT_NAME "AutoTrayIt_${ARCH_SUFFIX}_v${PROJECT_VERSION}_$<CONFIG>"
)

# 自定义命令：清理并复制生成的二进制文件
add_custom_command(TARGET AutoTrayIt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/build
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/build
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:AutoTrayIt> ${CMAKE_SOURCE_DIR}/build
)
