/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "common/task.h"
#include <string>

#define kFilePlugin "File Manager"
constexpr int kChunkSize = 1024*1024;

struct FileInfo {
    std::string name;
    std::string type;
    std::string size;
    std::string modified;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(FileInfo, name, type, size, modified);
};

enum FileStatus {
    SUCCESS,
    FAILED,
};

struct FileItem {
    std::string source_path;
    std::string target_folder;
    std::streamsize size;
    bool        is_upload;
};
