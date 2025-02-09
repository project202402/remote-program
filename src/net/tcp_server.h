/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "net/tcp_channel.h"
#include "common/macros_magic.h"

#include <cstdint>
#include <memory>
#include <string>

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        virtual void onNewConnection(std::unique_ptr<TcpChannel> channel) = 0;
    };

    void start(std::string ip, uint16_t port, Delegate* delegate);
    void stop();

    std::string ip() const;
    uint16_t port() const;

private:
    class Impl;
    std::shared_ptr<Impl> impl_;

    DISALLOW_COPY_AND_ASSIGN(TcpServer);
};
