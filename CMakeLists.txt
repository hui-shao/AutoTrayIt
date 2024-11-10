cmake_minimum_required(VERSION 3.10)
project(AutoTrayIt)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(thirdpart/tomlplusplus)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-finput-charset=UTF-8>")
add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-fexec-charset=UTF-8>")
add_compile_options("$<$<CXX_COMPILER_ID:Clang>:-finput-charset=UTF-8>")
add_compile_options("$<$<CXX_COMPILER_ID:Clang>:-fexec-charset=UTF-8>")

add_executable(AutoTrayIt WIN32 main.cpp)

target_link_libraries(AutoTrayIt PRIVATE user32 gdi32 shell32 tomlplusplus::tomlplusplus)