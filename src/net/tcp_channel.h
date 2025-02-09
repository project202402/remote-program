/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include "net/network_channel.h"
#include "net/variable_size.h"
#include "net/write_task.h"

#include <queue>
#include <asio/ip/tcp.hpp>
#include <asio/high_resolution_timer.hpp>


class TcpServer;

class TcpChannel final : public NetworkChannel
{
public:
    // Constructor available for Client.
    TcpChannel();
    ~TcpChannel() final;

    class Listener
    {
    public:
        virtual ~Listener() = default;

        virtual void onTcpConnected() = 0;
        virtual void onTcpDisconnected(ErrorCode error_code) = 0;
        virtual void onTcpMessageReceived(const ByteArray& buffer) = 0;
        virtual void onTcpMessageWritten(ByteArray&& buffer, size_t pending) = 0;
    };

    void setListener(Listener* listener);
    std::string peerAddress() const;
    void connect(std::string address, uint16_t port);
    bool isConnected() const;
    bool isPaused() const;
    void pause();
    void resume();
    void send(ByteArray&& buffer);
    bool setNoDelay(bool enable);
    bool setKeepAlive(bool enable,
                      const Seconds& interval = Seconds(60),
                      const Seconds& timeout = Seconds(30));

    bool setReadBufferSize(size_t size);
    bool setWriteBufferSize(size_t size);

    int getChannelId() const { return channel_id_; }
    void setChannelId(int channel_id) { channel_id_ = channel_id; }

protected:
    friend class TcpServer;

    explicit TcpChannel(asio::ip::tcp::socket&& socket);
    void disconnect();

private:

    enum class ReadState
    {
        IDLE,                // No reads are in progress right now.
        READ_SIZE,           // Reading the message size.
        READ_SERVICE_HEADER, // Reading the contents of the service header.
        READ_SERVICE_DATA,   // Reading the contents of the service data.
        READ_USER_DATA,      // Reading the contents of the user data.
        PENDING              // There is a message about which we did not notify.
    };

    enum ServiceMessageType
    {
        KEEP_ALIVE = 1
    };

    enum KeepAliveFlags
    {
        KEEP_ALIVE_PONG = 0,
        KEEP_ALIVE_PING = 1
    };

    struct ServiceHeader
    {
        uint8_t type;      // Type of service packet (see ServiceDataType).
        uint8_t flags;     // Flags bitmask (depends on the type).
        uint8_t reserved1; // Reserved.
        uint8_t reserved2; // Reserved.
        uint32_t length;   // Additional data size.
    };

    void onErrorOccurred(const std::error_code& error_code);
    void onErrorOccurred(ErrorCode error_code);

    void onResolved(const std::error_code& error_code,
                    const asio::ip::tcp::resolver::results_type& endpoints);
    void onConnected(const std::error_code& error_code, const asio::ip::tcp::endpoint& endpoint);

    void onMessageWritten(ByteArray&& buffer);
    void onMessageReceived();

    void addWriteTask(WriteTask::Type type, ByteArray&& data);

    void doWrite();
    void onWrite(const std::error_code& error_code, size_t bytes_transferred);

    void doReadSize();
    void onReadSize(const std::error_code& error_code, size_t bytes_transferred);

    void doReadUserData(size_t length);
    void onReadUserData(const std::error_code& error_code, size_t bytes_transferred);

    void doReadServiceHeader();
    void onReadServiceHeader(const std::error_code& error_code, size_t bytes_transferred);

    void doReadServiceData(size_t length);
    void onReadServiceData(const std::error_code& error_code, size_t bytes_transferred);

    void onKeepAliveInterval(const std::error_code& error_code);
    void onKeepAliveTimeout(const std::error_code& error_code);
    void sendKeepAlive(uint8_t flags, const void* data, size_t size);

    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    std::unique_ptr<asio::ip::tcp::resolver> resolver_;

    std::unique_ptr<asio::high_resolution_timer> keep_alive_timer_;
    Seconds keep_alive_interval_;
    Seconds keep_alive_timeout_;
    ByteArray keep_alive_counter_;
    TimePoint keep_alive_timestamp_;

    Listener* listener_ = nullptr;
    bool connected_ = false;
    bool paused_ = true;

    std::queue<WriteTask> write_queue_;
    VariableSizeWriter variable_size_writer_;
    ByteArray write_buffer_;

    ReadState state_ = ReadState::IDLE;
    VariableSizeReader variable_size_reader_;
    ByteArray read_buffer_;
    int channel_id_;

    class Handler;
    std::shared_ptr<Handler> handler_;

    DISALLOW_COPY_AND_ASSIGN(TcpChannel);
};