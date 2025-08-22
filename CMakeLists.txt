cmake_minimum_required(VERSION 4.0.3)
project(yerp)
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

add_executable(yerp src/main.cc src/glad.c src/glad.h)

target_link_libraries(yerp glfw OpenGL::GL)
