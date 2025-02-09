/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "plugins/host/host_impl.h"
#include <message_loop/message_loop.h>
#include "common/logger.h"

#include <asio.hpp>
#include <Windows.h>

std::string operatingSystemName()
{
    return "windows";
}

std::string computerName()
{
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD buffer_size = ARRAYSIZE(buffer);

    if (!GetComputerName(buffer, &buffer_size))
    {
        ZLOG_ERROR << "GetComputerNameW failed";
        return "";
    }

    return buffer;
}

std::string onlineTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string localIPv4() {
    asio::io_context ioContext;
    asio::ip::udp::resolver resolver(ioContext);
    asio::ip::udp::resolver::results_type endpoints = resolver.resolve(asio::ip::host_name(), "");

    for (const auto& endpoint : endpoints) {
        auto address = endpoint.endpoint().address();
        if(address.is_v4()) {
            return address.to_string();
        }
    }
    return "Error retrieving local IP.";
}