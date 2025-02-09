/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

class WriteTask
{
public:
    enum class Type { SERVICE_DATA, USER_DATA };
    using ByteArray = std::vector<uint8_t>;
    WriteTask(Type type, ByteArray&& data)
        : type_(type),
          data_(std::move(data))
    {
        // Nothing
    }

    Type type() const { return type_; }
    const ByteArray& data() const { return data_; }
    ByteArray& data() { return data_; }

private:
    const Type type_;
    ByteArray data_;
};