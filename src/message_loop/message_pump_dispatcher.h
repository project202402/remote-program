/*
* Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

class MessagePumpDispatcher
{
public:
    virtual ~MessagePumpDispatcher() = default;
    virtual bool draw() = 0;
    virtual void wakeup() = 0;
    virtual void waitEvents(double timeout) = 0;
};
