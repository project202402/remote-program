/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "plugins/file/file_control.h"
#include "plugins/file/file_impl.h"
#include "common/logger.h"
#include "common/encoding.h"
#include "common/task_worker.h"

EXPORT_CLASS(FileControl)
EXPORT_DELETEOBJECT

FileControl::FileControl() {
    bind("driveList", [this] {
        return listDrives();
    });

    bind("fileList", [this](const std::string& path) {
        auto gbk_str = EncodingConversion::UTF8ToGBK(path);
        return listFiles(gbk_str);
    });

    bind("startUpload", [this](const std::string& path, std::streamsize size) {
        auto gbk_str = EncodingConversion::UTF8ToGBK(path);
        downloader = std::make_unique<FileDownloader>(gbk_str, size);
        return SUCCESS;
    });

    bind("uploadFile", [this](const std::vector<char>& data) {
        downloader->writeChunk(data);
        if(downloader->offset() >= downloader->size()) {
            downloader.reset();
        }
        return SUCCESS;
    });

    bind("startDownload", [this](const std::string& path) {
        auto gbk_str = EncodingConversion::UTF8ToGBK(path);
        uploader = std::make_unique<FileUploader>(gbk_str, kChunkSize);
        return SUCCESS;
    });

    bind("downloadFile", [this] {
        auto chunk = uploader->readChunk();
        if(chunk.empty()) {
            uploader.reset();
        }
        return chunk;
    });
}

FileControl::~FileControl() {
}