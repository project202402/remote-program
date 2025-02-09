/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */


#include "net/session.h"

#include <memory>
#include <utility>

Session::Session(
    int channel_id,
    std::unique_ptr<TcpChannel> channel,
    std::shared_ptr<TaskRunner> io_task_runner):
        channel_id_(channel_id), channel_(std::move(channel)),
        task_runner_(std::move(io_task_runner))
{
    channel_->setListener(this);
}

Session::~Session()
{
    stop();
}

void Session::setListener(Session::Listener *listener) {
    listener_ = listener;
}

#include "../plugins/host/host_impl.h"
void Session::start() {
    static const size_t kReadBufferSize = 2 * 1024 * 1024;  // 2 Mb.
    channel_->setReadBufferSize(kReadBufferSize);
    channel_->setNoDelay(true);
    channel_->setKeepAlive(true);
    channel_->setListener(this);
    channel_->resume();

    task_runner_->postTask([this]{
        onTcpConnected();
    });
}

void Session::stop() {
    channel_->setListener(nullptr);
    channel_.reset();
}


int Session::id() const {
    return channel_id_;
}

void Session::send(ByteArray&& buffer) {
    return channel_->send(std::move(buffer));
}


void Session::onTcpConnected() {
    if (listener_)
        listener_->onSessionStarted(channel_id_);
}

void Session::onTcpDisconnected(NetworkChannel::ErrorCode error_code) {
    if (listener_)
        listener_->onSessionStop(channel_id_);
}

void Session::onTcpMessageReceived(const ByteArray &buffer) {
    if (listener_)
        listener_->onSessionMessageReceived(channel_id_, buffer);
}

void Session::onTcpMessageWritten(ByteArray &&buffer, size_t pending) {
    if (listener_)
        listener_->onSessionMessageWritten(channel_id_, pending);
}
