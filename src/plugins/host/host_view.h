/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "plugins/host/host_define.h"
#include "manager/setting_manager.h"
#include "common/view.h"
#include "imgui.h"

class HostView final : public View {
public:
    void draw() override;
    void onTaskDone(Task task) override;

private:
    void getHostInfo(int session_id);
    void onOffline(int session_id);
    void drawTable();
    void drawStatusBar();
    void doMenuTasks(const std::string& name, const ImVector<int>& selection);
    void startMenuTasks(const std::string& name, int session_id);
    void doUninstall(const ImVector<int>& selection);

    std::map<int, HostInfo> servers_;
    int listen_port_ = SettingManager::getInstance().getField<int>("listen_port");
    json plugins_ = SettingManager::getInstance().getField<json>("plugins");

};
