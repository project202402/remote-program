/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "plugins/host/host_control.h"

#include "plugins/host/host_define.h"
#include "plugins/host/host_impl.h"

#include "common/logger.h"
#include "common/task_worker.h"

#include "MemoryModulePP/MemoryModulePP.h"
#include <manager/control_manager.h>

HostControl::HostControl() {
    bind("getHostInfo", [this]{
        return this->doSysInfo();
    });
    bind("uninstall", [this]{
        getTaskRunner()->postQuit();
    });

#ifdef MEMORYLOAD
    bind("checkPlugin", [this](const std::string& name){
        return this->checkPlugin(name);
    });

    bind("loadPlugin", [this](const std::string& name, const std::vector<char> &data) {
        return this->loadPlugin(name, data);
    });

    bind("unloadPlguin", [this](const std::string& name) {
        return this->unloadPlguin(name);
    });
#endif

}

HostControl::~HostControl() {
}

HostInfo HostControl::doSysInfo() {
    HostInfo sys_info;
    sys_info.lan = localIPv4();
    sys_info.computer = computerName();
    sys_info.os = operatingSystemName();
    sys_info.online_time = onlineTime();
    return sys_info;
}

#ifdef MEMORYLOAD
bool HostControl::checkPlugin(const std::string &name) {
    auto it = ControlManager::factory.find(name);
    return it != ControlManager::factory.end();
}

bool HostControl::loadPlugin(const std::string &name, const std::vector<char> &data) {
    if(auto handle = MemoryLoadLibrary(data.data(), data.size())) {
        ControlManager::regist(name,
            [handle] {
                auto creator = reinterpret_cast<ControlObjectPtr>(MemoryGetProcAddress(handle, "createControlObject"));
                return creator();
        }, [handle](Control* control ) {
                auto deletor = reinterpret_cast<DeleteControlObjectPtr>(MemoryGetProcAddress(handle, "deleteControlObject"));
            deletor(control);
        });
        return true;
    }
    return false;
}

bool HostControl::unloadPlguin(const std::string &name) {
    return false;
}
#endif