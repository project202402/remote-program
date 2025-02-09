/*
* Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "net/tcp_server.h"
#include "net/tcp_channel.h"
#include "message_loop/message_loop.h"
#include "common/logger.h"
#include <asio/ip/address.hpp>

class TcpServer::Impl : public std::enable_shared_from_this<Impl>
{
public:
    explicit Impl(asio::io_context& io_context);
    ~Impl();

    void start(std::string ip, uint16_t port, Delegate* delegate);
    void stop();

    std::string ip() const;
    uint16_t port() const;

private:
    void doAccept();
    void onAccept(const std::error_code& error_code, asio::ip::tcp::socket socket);

    asio::io_context& io_context_;
    std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;
    Delegate* delegate_ = nullptr;

    std::string ip_;
    uint16_t port_ = 0;

    int accept_error_count_ = 0;

    DISALLOW_COPY_AND_ASSIGN(Impl);
};

//--------------------------------------------------------------------------------------------------
TcpServer::Impl::Impl(asio::io_context& io_context)
    : io_context_(io_context)
{
    ZLOG_INFO << "Ctor";
}

//--------------------------------------------------------------------------------------------------
TcpServer::Impl::~Impl()
{
    ZLOG_INFO << "Dtor";
    assert(!acceptor_);
}

//--------------------------------------------------------------------------------------------------
void TcpServer::Impl::start(std::string ip, uint16_t port, Delegate* delegate)
{
    delegate_ = delegate;
    ip_ = ip;
    port_ = port;

    assert(delegate_);

    ZLOG_INFO << "Listen ip: "
                 << (ip_.empty() ? "ANY" : ip_) << ":" << port;

    asio::ip::address listen_address;
    asio::error_code error_code;

    if (!ip_.empty())
    {
        listen_address = asio::ip::make_address(ip, error_code);
        if (error_code)
        {
            ZLOG_ERROR << "Invalid listen address: " << ip_
                          << " (" << error_code.message() << ")";
            return;
        }
    }
    else
    {
        listen_address = asio::ip::address_v6::any();
    }

    asio::ip::tcp::endpoint endpoint(listen_address, port_);
    acceptor_ = std::make_unique<asio::ip::tcp::acceptor>(io_context_);

    acceptor_->open(endpoint.protocol(), error_code);
    if (error_code)
    {
        ZLOG_ERROR << "acceptor_->open failed: "
                      << error_code.message();
        return;
    }

    acceptor_->set_option(asio::ip::tcp::acceptor::reuse_address(true), error_code);
    if (error_code)
    {
        ZLOG_ERROR << "acceptor_->set_option failed: "
                      << error_code.message();
        return;
    }

    acceptor_->bind(endpoint, error_code);
    if (error_code)
    {
        ZLOG_ERROR << "acceptor_->bind failed: "
                      << error_code.message();
        return;
    }

    acceptor_->listen(asio::ip::tcp::socket::max_listen_connections, error_code);
    if (error_code)
    {
        ZLOG_ERROR << "acceptor_->listen failed: "
                      << error_code.message();
        return;
    }

    doAccept();
}

//--------------------------------------------------------------------------------------------------
void TcpServer::Impl::stop()
{
    delegate_ = nullptr;
    acceptor_.reset();
}

//--------------------------------------------------------------------------------------------------
std::string TcpServer::Impl::ip() const
{
    return ip_;
}

//--------------------------------------------------------------------------------------------------
uint16_t TcpServer::Impl::port() const
{
    return port_;
}

//--------------------------------------------------------------------------------------------------
void TcpServer::Impl::doAccept()
{
    acceptor_->async_accept(
        std::bind(&Impl::onAccept, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

//--------------------------------------------------------------------------------------------------
void TcpServer::Impl::onAccept(const std::error_code& error_code, asio::ip::tcp::socket socket)
{
    if (!delegate_)
        return;

    if (error_code)
    {
        ZLOG_ERROR << "Error while accepting connection: "
                      << error_code.message();

        static const int kMaxErrorCount = 500;

        ++accept_error_count_;
        if (accept_error_count_ > kMaxErrorCount)
        {
            ZLOG_ERROR << "WARNING! Too many errors when trying to accept a connection. "
                          << "New connections will not be accepted";
            return;
        }
    }
    else
    {
        accept_error_count_ = 0;

        std::unique_ptr<TcpChannel> channel =
            std::unique_ptr<TcpChannel>(new TcpChannel(std::move(socket)));

        // Connection accepted.
        delegate_->onNewConnection(std::move(channel));
    }

    // Accept next connection.
    doAccept();
}

//--------------------------------------------------------------------------------------------------
TcpServer::TcpServer()
    : impl_(std::make_shared<Impl>(MessageLoop::current()->pumpAsio()->ioContext()))
{
    ZLOG_INFO << "Ctor";
}

//--------------------------------------------------------------------------------------------------
TcpServer::~TcpServer()
{
    ZLOG_INFO << "Dtor";
    impl_->stop();
}

//--------------------------------------------------------------------------------------------------
void TcpServer::start(std::string ip, uint16_t port, Delegate* delegate)
{
    impl_->start(ip, port, delegate);
}

//--------------------------------------------------------------------------------------------------
void TcpServer::stop()
{
    impl_->stop();
}

//--------------------------------------------------------------------------------------------------
std::string TcpServer::ip() const
{
    return impl_->ip();
}

//--------------------------------------------------------------------------------------------------
uint16_t TcpServer::port() const
{
    return impl_->port();
}

