/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include <chrono>
#include <cstdint>


#if defined(OS_WIN)
using NativeSocket = uintptr_t;
#elif defined(OS_POSIX)
using NativeSocket = int;
#else
#error Not implemented
#endif

bool setTcpKeepAlive(NativeSocket socket,
                     bool enable,
                     const std::chrono::milliseconds& time,
                     const std::chrono::milliseconds& interval);

