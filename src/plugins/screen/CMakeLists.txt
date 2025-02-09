cmake_minimum_required(VERSION 3.20)

add_library(screen
        screen_control.cpp
        screen_view.cpp
        win_screen.cpp
)

add_library(screen_control SHARED screen_control.cpp win_screen.cpp)
set_target_properties(screen_control PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/plugins
)
add_dependencies(screen screen_control)