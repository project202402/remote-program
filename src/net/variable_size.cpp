/*
* Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "net/variable_size.h"
#include "common/logger.h"

//--------------------------------------------------------------------------------------------------
VariableSizeReader::VariableSizeReader() = default;

//--------------------------------------------------------------------------------------------------
VariableSizeReader::~VariableSizeReader() = default;

//--------------------------------------------------------------------------------------------------
asio::mutable_buffer VariableSizeReader::buffer()
{
    assert(pos_ < std::size(buffer_));

    return asio::mutable_buffer(&buffer_[pos_], sizeof(uint8_t));
}

//--------------------------------------------------------------------------------------------------
std::optional<size_t> VariableSizeReader::messageSize()
{
    assert(pos_ < std::size(buffer_));

    if (pos_ == 3 || !(buffer_[pos_] & 0x80))
    {
        size_t result = buffer_[0] & 0x7F;

        if (pos_ >= 1)
            result += (buffer_[1] & 0x7F) << 7;

        if (pos_ >= 2)
            result += (buffer_[2] & 0x7F) << 14;

        if (pos_ >= 3)
            result += buffer_[3] << 21;

        memset(buffer_, 0, std::size(buffer_));
        pos_ = 0;

        return result;
    }
    else
    {
        ++pos_;

        return std::nullopt;
    }
}

//--------------------------------------------------------------------------------------------------
VariableSizeWriter::VariableSizeWriter() = default;

//--------------------------------------------------------------------------------------------------
VariableSizeWriter::~VariableSizeWriter() = default;

//--------------------------------------------------------------------------------------------------
asio::const_buffer VariableSizeWriter::variableSize(size_t size)
{
    size_t length = 1;

    buffer_[0] = size & 0x7F;
    if (size > 0x7F) // 127 bytes
    {
        buffer_[0] |= 0x80;
        buffer_[length++] = size >> 7 & 0x7F;

        if (size > 0x3FFF) // 16383 bytes
        {
            buffer_[1] |= 0x80;
            buffer_[length++] = size >> 14 & 0x7F;

            if (size > 0x1FFFF) // 2097151 bytes
            {
                buffer_[2] |= 0x80;
                buffer_[length++] = size >> 21 & 0xFF;
            }
        }
    }

    return asio::const_buffer(buffer_, length);
}