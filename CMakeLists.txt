cmake_minimum_required(VERSION 3.10)
project(sim)

cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)
find_package(Kokkos REQUIRED)

file(GLOB_RECURSE sources src/*.cc)

# ImGui setup
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/src/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Glad source (in src/)
set(GLAD_SOURCE ${CMAKE_SOURCE_DIR}/src/glad.c)

add_executable(sim
    ${sources}
    ${IMGUI_SOURCES}
    ${GLAD_SOURCE}
)

target_include_directories(sim PRIVATE
    ${CMAKE_SOURCE_DIR}/src    # so "glad.h" is found
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(sim PRIVATE
  Kokkos::kokkos
    glfw
    OpenGL::GL
)

