/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "net/tcp_channel.h"
#include "common/task_runner.h"
#include "common/task_consumer.h"
#include "common/task_producer.h"
#include "common/thread.h"

class Server final :
    public TcpChannel::Listener,
    public TaskConsumer,
    public TaskProducer
{

public:
    Server(const std::string& ip, int port);
    ~Server() override = default;

    void run();
    std::shared_ptr<TaskRunner> taskRunner() const;

private:
    void doTask(Task task) override;
    void onTaskDone(Task task) override;
    bool ensureControlExists(int view_id, const std::string& plugin);

    void connect();

    //TcpChannel::Listener
    void onTcpConnected() override;
    void onTcpDisconnected(NetworkChannel::ErrorCode error_code) override;
    void onTcpMessageReceived(const ByteArray& buffer) override;
    void onTcpMessageWritten(ByteArray&& buffer, size_t pending) override;

    std::unique_ptr<TcpChannel> channel_;
    MessageLoop *message_loop_ = nullptr;
    std::string server_ip_;
    int server_port_;
};