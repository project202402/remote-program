/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "plugins/shell/win_shell.h"
#include "plugins/shell/shell_define.h"
#include "common/logger.h"

#include <chrono>
#include <utility>

WinShell::WinShell() {
}

WinShell::~WinShell() {
    stop();
}

bool WinShell::start() {

    HANDLE hPipePTYIn = INVALID_HANDLE_VALUE;
    HANDLE hPipePTYOut = INVALID_HANDLE_VALUE;

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&out, &hPipePTYOut, &saAttr, 0)) {
        return false;
    }

    if (!CreatePipe(&hPipePTYIn, &in, &saAttr, 0)) {
        return false;
    }

    TCHAR szComspec[MAX_PATH];
    if (GetEnvironmentVariable(TEXT("COMSPEC"), szComspec, MAX_PATH) == 0) {
        return false;
    }

    ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));

    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.hStdOutput = hPipePTYOut;
    startupInfo.hStdInput = hPipePTYIn;
    startupInfo.wShowWindow = SW_HIDE;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

    HRESULT hr = CreateProcess(
        NULL,                             // No module name - use Command Line
        szComspec,                        // Command Line
        NULL,                             // Process handle not inheritable
        NULL,                             // Thread handle not inheritable
        TRUE,                             // Inherit handles
        0,                                // Creation flags
        NULL,                             // Use parent's environment block
        NULL,                             // Use parent's starting directory
        &startupInfo,                     // Pointer to STARTUPINFO
        &procInfo                         // Pointer to PROCESS_INFORMATION
    ) ? S_OK : HRESULT_FROM_WIN32(GetLastError());

    if (hr != S_OK) {
        CloseHandle(hPipePTYIn);
        CloseHandle(hPipePTYOut);
        return false;
    }

    CloseHandle(hPipePTYIn);
    CloseHandle(hPipePTYOut);
    return true;
}

void WinShell::stop() {

    running = false;
    if (readerThread.joinable()) {
        readerThread.join();
    }
    terminate();
    cleanup();
}


bool WinShell::read()  {
    char readBuf[SHELL_DATA_MAX_LENGTH - sizeof(uint32_t)] = { 0 };
    DWORD bytesRead = 0;
    DWORD bytesAvailable = 0;

    if (!PeekNamedPipe(out, NULL, 0, NULL, &bytesAvailable, NULL) || bytesAvailable == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return true;
    }

    if (!ReadFile(out, readBuf, sizeof(readBuf), &bytesRead, NULL) || bytesRead == 0) {
        return false;
    }

    std::string buf(readBuf, bytesRead);
    callback_(buf);

    return true;
}


bool WinShell::write(const std::string& data) {
    const char *buf = data.c_str();
    size_t length = data.length();
    DWORD bytesWritten;
    while (length > 0) {
        if (!WriteFile(in, buf, static_cast<DWORD>(length), &bytesWritten, NULL)) {
            stop();
            return false;
        }

        if (bytesWritten == 0) break;

        length -= bytesWritten;
        buf += bytesWritten;
    }
    return true;
}

void WinShell::setCallback(std::function<void(const std::string& )> callback) {
    callback_ = std::move(callback);
    running = true;

    readerThread = std::thread([this]() {
        while (running) {
            if (!read()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    });
}

std::function<void(const std::string& )> WinShell::getCallback() {
    return callback_;
}


void WinShell::terminate() {
    if (procInfo.hProcess != INVALID_HANDLE_VALUE) {
        TerminateProcess(procInfo.hProcess, 0);
        WaitForSingleObject(procInfo.hProcess, INFINITE);

        CloseHandle(procInfo.hProcess);
        CloseHandle(procInfo.hThread);
        procInfo.hProcess = INVALID_HANDLE_VALUE;
        procInfo.hThread = INVALID_HANDLE_VALUE;
    }
}

void WinShell::cleanup() {
    if (in != INVALID_HANDLE_VALUE) {
        CloseHandle(in);
        in = INVALID_HANDLE_VALUE;
    }
    if (out != INVALID_HANDLE_VALUE) {
        CloseHandle(out);
        out = INVALID_HANDLE_VALUE;
    }
}


