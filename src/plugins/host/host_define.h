/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "common/task.h"
#include <string>
#define kHostInfoPlugin "Host Info"

enum
{
    ONLINE,
    OFFLINE,
};

struct HostInfo
{
    std::string lan;
    std::string computer;
    std::string os;
    std::string online_time;
    std::string notes;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(HostInfo, lan, computer, os, online_time, notes);
};
