cmake_minimum_required(VERSION 3.20)

add_library(file_control SHARED file_control.cpp file_impl.cpp)
add_library(file
        file_control.cpp
        file_impl.cpp
        file_view.cpp
)
target_link_libraries(file_control common)
set_target_properties(file_control PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/plugins
)
add_dependencies(file file_control)