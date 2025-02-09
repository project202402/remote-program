# IMGUI
include_directories(
        ${CMAKE_SOURCE_DIR}/3rd
        ${CMAKE_SOURCE_DIR}/3rd/imgui
        ${CMAKE_SOURCE_DIR}/3rd/imgui/backends
        ${CMAKE_SOURCE_DIR}/3rd/imgui/examples/libs/glfw/include
)

# STB
include_directories(${CMAKE_SOURCE_DIR}/3rd/stb)

# ImGuiFileDialog
include_directories(${CMAKE_SOURCE_DIR}/3rd/ImGuiFileDialog)

# ASIO
include_directories(${CMAKE_SOURCE_DIR}/3rd/asio-1.30.2/asio/include)

# JSON
include_directories(${CMAKE_SOURCE_DIR}/3rd/json/single_include)

# MemoryModulePP
include_directories(${CMAKE_SOURCE_DIR}/3rd/MemoryModulePP)

add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)

# 设置imgui的相关源文件
set(IMGUI_SRC
        ${CMAKE_SOURCE_DIR}/3rd/imgui/imgui.cpp
        ${CMAKE_SOURCE_DIR}/3rd/imgui/imgui_demo.cpp
        ${CMAKE_SOURCE_DIR}/3rd/imgui/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/3rd/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_SOURCE_DIR}/3rd/imgui/backends/imgui_impl_opengl3.cpp
        ${CMAKE_SOURCE_DIR}/3rd/imgui/imgui_tables.cpp
        ${CMAKE_SOURCE_DIR}/3rd/imgui/imgui_widgets.cpp
)

set(MEMORYMODULE_SRC
        ${CMAKE_SOURCE_DIR}/3rd/MemoryModulePP/MemoryModulePP.c
)

# ImGuiFileDialog
set(IMGUI_FILE_DIALOG_SRC
        ${CMAKE_SOURCE_DIR}/3rd/ImGuiFileDialog/ImGuiFileDialog.cpp
)

find_library(GLFW3_LIB glfw3 PATHS ${CMAKE_SOURCE_DIR}/3rd/imgui/examples/libs/glfw/lib-vc2010-64)
find_package(OpenGL REQUIRED)

set(THIRD_PARTY_LIBS
        ${GLFW3_LIB}
        OpenGL::GL
        opengl32
)
