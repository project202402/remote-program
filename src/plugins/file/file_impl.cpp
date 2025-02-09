/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "plugins/file/file_impl.h"
#include "plugins/file/file_define.h"
#include "common/encoding.h"

#include <windows.h>

#include <string>
#include <vector>
#include <filesystem>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
namespace fs = std::filesystem;

std::vector<FileInfo> listDrives() {
    DWORD drives = GetLogicalDrives();
    if (drives == 0) {
        return {};
    }

    std::vector<FileInfo> driveList{};
    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        if (drives & 1) {
            FileInfo driveInfo {};
            driveInfo.name = std::string(1, drive) + ":\\";
            UINT driveType = GetDriveType(driveInfo.name.c_str());
            switch (driveType) {
                case DRIVE_UNKNOWN:
                    driveInfo.type = "Unknown";
                break;
                case DRIVE_NO_ROOT_DIR:
                    driveInfo.type = "No Root Directory";
                break;
                case DRIVE_REMOVABLE:
                    driveInfo.type = "Removable";
                break;
                case DRIVE_FIXED:
                    driveInfo.type = "Fixed";
                break;
                case DRIVE_REMOTE:
                    driveInfo.type = "Network Drive";
                break;
                case DRIVE_CDROM:
                    driveInfo.type = "CD-ROM";
                break;
                case DRIVE_RAMDISK:
                    driveInfo.type = "RAM Disk";
                break;
                default:
                    driveInfo.type = "Error: Unknown type";
                break;
            }
            driveInfo.size = "-";
            driveInfo.modified = "-";
            driveList.emplace_back(driveInfo);
        }
        drives >>= 1;
    }
    return driveList;
}


static std::time_t to_time_t(const FILETIME& ftime) {
    ULARGE_INTEGER ull;
    ull.LowPart = ftime.dwLowDateTime;
    ull.HighPart = ftime.dwHighDateTime;
    return static_cast<std::time_t>((ull.QuadPart - 116444736000000000ULL) / 10000000ULL);
}

static FileInfo listFile(const std::string& filePath) {
    FileInfo file_info;
    WIN32_FILE_ATTRIBUTE_DATA fileInfoData;

    if (GetFileAttributesEx(filePath.c_str(), GetFileExInfoStandard, &fileInfoData)) {
        file_info.name = EncodingConversion::GBKToUTF8(filePath.substr(filePath.find_last_of("\\/") + 1));
        if (fileInfoData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            file_info.type = "Directory";
            file_info.size = "-";
        } else {
            file_info.type = "File";
            file_info.size = std::to_string((fileInfoData.nFileSizeHigh * (MAXDWORD + 1)) + fileInfoData.nFileSizeLow);
        }

        auto ftime = to_time_t(fileInfoData.ftLastWriteTime);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&ftime), "%F %T");
        file_info.modified = oss.str();
    } else {
        std::cerr << "Error getting file attributes: " << GetLastError() << std::endl;
    }

    return file_info;
}

std::vector<FileInfo> listFiles(const std::string& path) {
    std::vector<FileInfo> files;
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((path + "\\*").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string fileName = findFileData.cFileName;
            if (fileName != "." && fileName != "..") {
                files.emplace_back(listFile(path + "\\" + fileName));
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    } else {
        std::cerr << "Error finding files: " << GetLastError() << std::endl;
    }

    return files;
}

std::streamsize getFileSize(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    // 获取文件大小
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    return fileSize;
}