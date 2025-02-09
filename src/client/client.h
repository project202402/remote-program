/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include "common/task_runner.h"
#include "common/task_consumer.h"
#include "common/task_producer.h"
#include "common/thread.h"
#include <memory>

class View;
class Session;
class TcpChannel;
class ViewManager;
class SessionManager;

class Client final :
        public TaskProducer,
        public TaskConsumer
{
 public:
    Client();
    ~Client() override;

    std::shared_ptr<TaskRunner> uiTaskRunner();
    std::shared_ptr<TaskRunner> ioTaskRunner();

    void run();
    void quit();

    void doTask(Task request) override;
    void onTaskDone(Task response) override;

 private:
    void initIOTasks();
    void initUITasks();
    void cleanupTasks();

    MessageLoop*                    message_loop_ = nullptr;
    Thread                          io_thread_;
    std::shared_ptr<TaskRunner>     io_task_runner_;
    std::unique_ptr<SessionManager> session_manager_;

    DISALLOW_COPY_AND_ASSIGN(Client);
};