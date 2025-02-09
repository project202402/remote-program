/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "plugins/file/file_define.h"
#include "plugins/file/file_explorer.h"
#include "plugins/file/file_handler.h"
#include "common/view.h"

#include <queue>

class FileView : public View {
public:
    void draw() override;
    void start() override;

private:
    void driverList();
    void fileList(const std::string& path);

    void startUpload();
    void uploadFile();
    void startDownload();
    void downloadFile();
    void uploadFiles(const std::vector<FileItem>& items);
    void downloadFiles(const std::vector<FileItem>& items);

    FileExplorer file_explore_;
    std::unique_ptr<FileUploader>   uploader_;
    std::unique_ptr<FileDownloader> downloader_;
    std::queue<FileItem> upload_items_;
    std::queue<FileItem> download_items_;

    std::string remote_path_;
    std::vector<FileInfo> remote_files_;
};