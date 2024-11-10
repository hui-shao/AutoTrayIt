cmake_minimum_required(VERSION 3.10)
project(AutoTrayIt)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(thirdpart/tomlplusplus)

add_executable(AutoTrayIt WIN32 main.cpp)

target_link_libraries(AutoTrayIt PRIVATE user32 gdi32 shell32 tomlplusplus::tomlplusplus)