cmake_minimum_required(VERSION 3.20)
project(server)

add_definitions(-DSERVER_END)

add_executable(server
        ${MEMORYMODULE_SRC}
        server.cpp
        server.h
        main.cpp
)

set_target_properties(server PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

if(NOT DEFINED MEMORYLOAD)
    set(ADDITIONAL_LIB file screen shell)
endif()
target_link_libraries(server common manager message_loop net utils host ${ADDITIONAL_LIB})
