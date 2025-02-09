/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "plugins/file/file_define.h"
#include <vector>

std::vector<FileInfo> listDrives() ;
std::vector<FileInfo> listFiles(const std::string& path);
std::streamsize getFileSize(const std::string& filePath);

