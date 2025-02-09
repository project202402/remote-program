/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "client/client.h"
#include "client/win/imgui_window.h"
#include "manager/session_manager.h"
#include "manager/view_manager.h"
#include "manager/setting_manager.h"
#include "plugins/host/host_define.h"
#include "common/logger.h"


Client::Client()
{
    ZLOG_INFO << "Ctor";

    io_thread_.start(MessageLoop::Type::ASIO);
    io_task_runner_ = io_thread_.taskRunner();
}

Client::~Client()
{
    io_thread_.stop();
}

std::shared_ptr<TaskRunner> Client::uiTaskRunner()
{
    return message_loop_->taskRunner();
}

std::shared_ptr<TaskRunner> Client::ioTaskRunner()
{
    return io_task_runner_;
}

void Client::run() {
    MessageLoop message_loop(MessageLoop::Type::UI);
    message_loop_ = &message_loop;

    initIOTasks();
    initUITasks();

    auto imgui_window = std::make_unique<ImguiWindow>(uiTaskRunner());
    message_loop_->run(imgui_window.get());

    cleanupTasks();
}

void Client::initIOTasks() {
    ioTaskRunner()->postTask([this] {
        session_manager_ = std::make_unique<SessionManager>(ioTaskRunner());
        session_manager_->setTaskConsumer(this);

        int listen_port = SettingManager::getInstance().getField<int>("listen_port");
        session_manager_->start(listen_port);
    });
}

void Client::initUITasks() {
    uiTaskRunner()->postTask([this] {
        auto main_id = ViewManager::create(kHostInfoPlugin);
        auto &main_view = ViewManager::views[main_id];
        main_view->setTaskRunner(uiTaskRunner());
        main_view->setTaskProducer(this);
    });
}

void Client::cleanupTasks() {
    io_task_runner_->postTask([this] {
        session_manager_.reset();
    });
}

void Client::quit() {
    message_loop_->taskRunner()->postQuit();
}

void Client::doTask(Task request) {
    ioTaskRunner()->postTask([request, this]{
        const auto& session_id = request["session_id"];
        const auto& plugin = request["plugin"];
        const auto& command = request["command"];
        auto &channels = session_manager_->channels();
        auto it = channels.find(session_id);
        if (it != channels.end()) {
            return it->second->send(Task::to_bson(request));
        }
        ZLOG_ERROR << "channel_id not exist: " << command << "," <<
            plugin << "," << session_id;
    });
}

void Client::onTaskDone(Task response) {
    uiTaskRunner()->postTask([response]() mutable{
        auto& view_id = response["view_id"];
        const auto& plugin = response["plugin"];
        const auto& command = response["command"];
        auto &views = ViewManager::views;
        auto it = views.find(view_id);
        if (it != views.end()) {
            return it->second->onTaskDone(response);
        }
        ZLOG_ERROR << "view_id not exist: " << command << "," <<
            plugin << "," << view_id;
    });
}
