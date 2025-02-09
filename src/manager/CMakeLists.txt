cmake_minimum_required(VERSION 3.20)

add_library(manager
        control_manager.cpp
        session_manager.cpp
        view_manager.cpp
        setting_manager.cpp
        setting_manager.h
)