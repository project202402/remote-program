cmake_minimum_required(VERSION 3.20)
project(client)

add_definitions(-DCLIENT_END)

add_executable(client
        ${IMGUI_SRC}
        ${IMGUI_FILE_DIALOG_SRC}
        client.cpp
        main.cpp
        win/imgui_logger.cpp
        win/imgui_window.cpp
)

set_target_properties(client PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)
target_link_libraries(client common manager net file host screen shell ${THIRD_PARTY_LIBS} message_loop utils)