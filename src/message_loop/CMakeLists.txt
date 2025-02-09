cmake_minimum_required(VERSION 3.20)

add_definitions(-DCLIENT_END)
add_library(message_loop
        message_loop.cpp
        message_loop_task_runner.cpp
        message_pump_asio.cpp
        message_pump_default.cpp
        message_pump_ui.cpp
        pending_task.cpp
)
