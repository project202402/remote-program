/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "plugins/file/file_handler.h"
#include "common/control.h"
#include "common/scoped_task_runner.h"

class FileControl : public Control
{
public:
    FileControl();
    ~FileControl() override;

private:
    std::unique_ptr<FileDownloader>  downloader;
    std::unique_ptr<FileUploader>    uploader;
};
