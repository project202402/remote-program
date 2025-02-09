/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include <Windows.h>
#include <string>
#include <thread>
#include <functional>

class WinShell {
public:
    WinShell();
    ~WinShell();

    void setCallback(std::function<void(const std::string& )> callback);
    std::function<void(const std::string& )> getCallback();

    bool start();
    void stop();
    bool write(const std::string& data);

private:
    bool read();
    void terminate();
    void cleanup();

    std::function<void(const std::string&)> callback_;
    HANDLE in{ INVALID_HANDLE_VALUE }, out{ INVALID_HANDLE_VALUE };
    PROCESS_INFORMATION procInfo{};
    STARTUPINFO startupInfo{};
    std::thread readerThread;
    bool running = false;
};
