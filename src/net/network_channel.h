/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include <chrono>

using ByteArray = std::vector<uint8_t>;

class NetworkChannel
{
public:
    static const uint32_t kMaxMessageSize;
    using ByteArray = std::vector<uint8_t>;
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Milliseconds = std::chrono::milliseconds;
    using Seconds = std::chrono::seconds;

    enum class ErrorCode
    {
        // Unknown error.
        UNKNOWN,

        // No error.
        SUCCESS,

        // Violation of the communication protocol.
        INVALID_PROTOCOL,

        // Cryptography error (message encryption or decryption failed).
        ACCESS_DENIED,

        // An error occurred with the network (e.g., the network cable was accidentally plugged out).
        NETWORK_ERROR,

        // The connection was refused by the peer (or timed out).
        CONNECTION_REFUSED,

        // The remote host closed the connection.
        REMOTE_HOST_CLOSED,

        // The host address was not found.
        SPECIFIED_HOST_NOT_FOUND,

        // The socket operation timed out.
        SOCKET_TIMEOUT,

        // The address specified is already in use and was set to be exclusive.
        ADDRESS_IN_USE,

        // The address specified does not belong to the host.
        ADDRESS_NOT_AVAILABLE
    };

    virtual ~NetworkChannel() = default;

    int64_t totalRx() const { return total_rx_; }
    int64_t totalTx() const { return total_tx_; }
    int speedRx();
    int speedTx();

    // Converts an error code to a human readable string.
    // Does not support localization. Used for logs.
    static std::string errorToString(ErrorCode error_code);

protected:
    void addTxBytes(size_t bytes_count);
    void addRxBytes(size_t bytes_count);

    static void resizeBuffer(ByteArray* buffer, size_t new_size);

private:
    int64_t total_tx_ = 0;
    int64_t total_rx_ = 0;

    TimePoint begin_time_tx_;
    int64_t bytes_tx_ = 0;
    int speed_tx_ = 0;

    TimePoint begin_time_rx_;
    int64_t bytes_rx_ = 0;
    int speed_rx_ = 0;
};