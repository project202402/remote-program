/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "common/macros_magic.h"

#include <asio/buffer.hpp>

#include <cstdint>
#include <optional>

class VariableSizeReader
{
public:
    VariableSizeReader();
    ~VariableSizeReader();

    asio::mutable_buffer buffer();
    std::optional<size_t> messageSize();

private:
    uint8_t buffer_[4] = { 0 };
    size_t pos_ = 0;

    DISALLOW_COPY_AND_ASSIGN(VariableSizeReader);
};

class VariableSizeWriter
{
public:
    VariableSizeWriter();
    ~VariableSizeWriter();

    asio::const_buffer variableSize(size_t size);

private:
    uint8_t buffer_[4];

    DISALLOW_COPY_AND_ASSIGN(VariableSizeWriter);
};
