/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "message_loop/message_pump_dispatcher.h"
#include "manager/setting_manager.h"
#include "common/task_runner.h"

struct GLFWwindow;
class ImguiWindow : public MessagePumpDispatcher {
 public:
    explicit ImguiWindow(std::shared_ptr<TaskRunner>);
    ~ImguiWindow() override;

    bool draw() override;
    void wakeup() override;
    void waitEvents(double timeout) override;

 private:
    void drawInner();
    void drawMenus();
    void drawViews();
    void drawLogs();

    void drawSettings();
    void drawBuild();
    void drawAbout();

    std::shared_ptr<TaskRunner> task_runner_;
    GLFWwindow* window_ = nullptr;

    bool selected_light_ = true;
    bool selected_dark_ = false;
    bool selected_classic_ = false;

    bool open_settings_ = false;
    bool open_build_ = false;
    bool open_about_ = false;

    std::string server_ip_;
    int server_port_;
    std::string listen_ip_;
    int listen_port_;
    int idx_frame_ = 0;
};
