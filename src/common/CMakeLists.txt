cmake_minimum_required(VERSION 3.20)

add_library(common
        encoding.cpp
        scoped_task_runner.cpp
        task_runner.cpp
        task_worker.cpp
        thread.cpp
)