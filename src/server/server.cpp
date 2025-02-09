/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "server/server.h"

#include <plugins/host/host_control.h>
#include "common/task_worker.h"
#include "common/logger.h"
#include "manager/control_manager.h"

Server::Server(const std::string &ip, int port):
    server_ip_(ip), server_port_(port){
}

void Server::run() {
    MessageLoop message_loop(MessageLoop::Type::ASIO);
    message_loop_ = &message_loop;
    message_loop_->taskRunner()->postTask([this]{
        connect();
    });
    message_loop_->run();
    channel_.reset();
}

std::shared_ptr<TaskRunner> Server::taskRunner() const {
    if(message_loop_)
        return message_loop_->taskRunner();
    return nullptr;
}

void Server::doTask(Task task) {
    auto view_id = task["view_id"];
    auto plugin = task["plugin"];

    TaskWorker::taskRunner()->postTask([this, view_id, plugin, task] {
        auto& controls = ControlManager::controls;
        if (!ensureControlExists(view_id, plugin)) {
            ZLOG_ERROR << "ControlManager: failed to create view for " << view_id << " with plugin " << plugin;
            return;
        }
        auto& control = controls[view_id];
        control->doTask(task);
    });
}

bool Server::ensureControlExists(int view_id, const std::string& plugin) {
    auto& controls = ControlManager::controls;
    if (!controls.contains(view_id)) {
        if (!ControlManager::create(view_id, plugin)) {
            printf("hello");
            return false;
        }
        auto& control = controls[view_id];
        control->setTaskConsumer(this);
        control->setTaskRunner(message_loop_->taskRunner());
    }
    return true;
}

void Server::onTaskDone(Task task) {
    if(auto task_runner = taskRunner()) {
        task_runner->postTask([this, task] {
            channel_->send(Task::to_bson(task));
        });
    }
}

void Server::connect() {
    channel_ = std::make_unique<TcpChannel>();
    channel_->setListener(this);
    channel_->connect(server_ip_, server_port_);
}

void Server::onTcpConnected() {
    channel_->setNoDelay(true);
    channel_->setKeepAlive(true);
    channel_->resume();
    ZLOG_INFO << "onTcpConnetced";
}

void Server::onTcpDisconnected(NetworkChannel::ErrorCode error_code) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    ControlManager::controls.clear();
    connect();
}

void Server::onTcpMessageReceived(const ByteArray &buffer) {
    auto task = Task::from_bson(buffer);
    doTask(task);
}

void Server::onTcpMessageWritten(ByteArray &&buffer, size_t pending) {

}