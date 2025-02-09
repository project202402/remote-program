/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "manager/session_manager.h"
#include "common/view.h"
#include "common/task.h"
#include "common/logger.h"

#include <memory>
#include <utility>

#define ONLINE 0
#define OFFLINE 1

int SessionManager::count = 0;

SessionManager::SessionManager(std::shared_ptr<TaskRunner> io_task_runner):
        io_task_runner_(std::move(io_task_runner)),
        tcp_server_(std::make_unique<TcpServer>())
{
}

SessionManager::~SessionManager()
{
    tcp_server_->stop();
    io_task_runner_.reset();
}

void SessionManager::start(int port) {
    return tcp_server_->start("0.0.0.0", port, this);
}

void SessionManager::onNewConnection(std::unique_ptr<TcpChannel> channel)
{
    int session_id = ++count;
    auto session = std::make_unique<Session>(session_id, std::move(channel), io_task_runner_);
    session->setListener(this);
    session->start();
    channels_[session_id] = std::move(session);
}

const std::map<int, std::unique_ptr<Session>> &SessionManager::channels()
{
    return channels_;
}

void SessionManager::onSessionStarted(int channel_id)
{
    task_consumer_->onTaskDone(TaskFactory::task(0, channel_id, "session", ONLINE));
}

void SessionManager::onSessionMessageReceived(int channel_id, const ByteArray &buffer)
{
    auto data = Task::from_bson(buffer);
    task_consumer_->onTaskDone(data);
}

void SessionManager::onSessionMessageWritten(int channel_id, size_t pending)
{
}

void SessionManager::onSessionStop(int channel_id)
{
    task_consumer_->onTaskDone(TaskFactory::task(0, channel_id, "session", OFFLINE));
    channels_.erase(channel_id);
}

void SessionManager::setTaskConsumer(TaskConsumer *task_consumer)
{
    task_consumer_ = task_consumer;
}
