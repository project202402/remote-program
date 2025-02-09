/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */


#pragma once

#include "net/session.h"
#include "net/tcp_server.h"
#include "common/task_runner.h"
#include "common/task_consumer.h"

#include <map>
#include <memory>

class SessionManager :
        public TcpServer::Delegate,
        public Session::Listener
{
 public:
    explicit SessionManager(std::shared_ptr<TaskRunner> io_task_runner);
    ~SessionManager() override;

    void start(int port);
    void setTaskConsumer(TaskConsumer* task_consumer);
    const std::map<int, std::unique_ptr<Session>>& channels();

 private:
    //  TcpServer::Delegate
    void onNewConnection(std::unique_ptr<TcpChannel> channel) override;
    //  Session::Listener
    void onSessionStarted(int channel_id) override;
    void onSessionMessageReceived(int channel_id, const ByteArray& buffer) override;
    void onSessionMessageWritten(int channel_id, size_t pending) override;
    void onSessionStop(int channel_id) override;

    static int count;
    std::map<int, std::unique_ptr<Session>> channels_;
    std::shared_ptr<TaskRunner> io_task_runner_;
    std::unique_ptr<TcpServer>  tcp_server_;
    TaskConsumer* task_consumer_{};
};
