cmake_minimum_required(VERSION 3.20)

add_library(shell
        shell_control.cpp
        shell_view.cpp
        win_shell.cpp
)

add_library(shell_control SHARED shell_control.cpp win_shell.cpp)
set_target_properties(shell_control PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/plugins
)

add_dependencies(shell shell_control)