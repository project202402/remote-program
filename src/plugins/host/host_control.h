/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "plugins/host/host_define.h"
#include "common/control.h"

class HostControl : public Control
{
public:
    HostControl();
    ~HostControl() override;

private:
    HostInfo doSysInfo();

#ifdef MEMORYLOAD
    // plugins
    bool checkPlugin(const std::string& name);
    bool loadPlugin(const std::string &name, const std::vector<char> &pluginData);
    bool unloadPlguin(const std::string& name);
#endif

};

