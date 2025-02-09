cmake_minimum_required(VERSION 3.20)

add_library(net
        network_channel.cpp
        session.cpp
        tcp_channel.cpp
        tcp_server.cpp
        variable_size.cpp
)