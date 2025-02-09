/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "binary_overlay.h"

std::string BinaryOverlay::readOverlay() {
    char filePath[MAX_PATH];
    if (GetModuleFileNameA(nullptr, filePath, MAX_PATH) == 0) {
     return "";
    }

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
     return "";
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule == NULL) {
     return "";
    }

    auto* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER *>(hModule);
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
     return "";
    }

    auto* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS *>(reinterpret_cast<BYTE *>(hModule) + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
     return "";
    }
    auto* sectionHeader = reinterpret_cast<IMAGE_SECTION_HEADER *>(
        reinterpret_cast<BYTE *>(ntHeaders) + sizeof(IMAGE_NT_HEADERS));
    IMAGE_SECTION_HEADER* lastSection = &sectionHeader[ntHeaders->FileHeader.NumberOfSections - 1];
    DWORD lastSectionEnd = lastSection->PointerToRawData + lastSection->SizeOfRawData;
    if (fileSize > lastSectionEnd) {
     DWORD overlaySize = fileSize - lastSectionEnd;
     file.seekg(lastSectionEnd, std::ios::beg);

     std::string overlayData(overlaySize, '\0');
     if (file.read(overlayData.data(), overlaySize)) {
      return overlayData;
     }
    }
    file.close();
    return "";
}


bool BinaryOverlay::writeOverlay(const std::string &src, const std::string &dst, const std::string &overlayData)
{
    if (overlayData.empty()) {
        return false;
    }

    std::ifstream srcFile(src, std::ios::binary);
    if (!srcFile) {
        return false;
    }

    std::ofstream dstFile(dst, std::ios::binary);
    if (!dstFile) {
        return false;
    }

    constexpr std::size_t bufferSize = 8192;
    char buffer[bufferSize];
    while (srcFile.read(buffer, bufferSize)) {
        dstFile.write(buffer, srcFile.gcount());
    }
    if (srcFile.gcount() > 0) {
        dstFile.write(buffer, srcFile.gcount());
    }

    dstFile.write(overlayData.data(), overlayData.size());
    return true;
}
