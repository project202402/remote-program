/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "plugins/screen/win_screen.h"
#include "common/logger.h"

#include <atlimage.h>
#include <atlenc.h>

#pragma comment (lib, "Gdiplus.lib")

class GdiplusManager {
public:
    static void Initialize() {
        if (!initialized) {
            Gdiplus::GdiplusStartupInput startupInput;
            Gdiplus::GdiplusStartup(&token, &startupInput, nullptr);
            initialized = true;
        }
    }

    static void Shutdown() {
        if (initialized) {
            Gdiplus::GdiplusShutdown(token);
            initialized = false;
        }
    }

private:
    static ULONG_PTR token;
    static bool initialized;
};


ULONG_PTR GdiplusManager::token = 0;
bool GdiplusManager::initialized = false;

class Screenshot {
public:
    static std::unique_ptr<Gdiplus::Bitmap> Capture() {
        HDC desktopDC = GetDC(GetDesktopWindow());
        HDC memoryDC = CreateCompatibleDC(desktopDC);
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);
        HBITMAP hBitmap = CreateCompatibleBitmap(desktopDC, width, height);
        SelectObject(memoryDC, hBitmap);
        BitBlt(memoryDC, 0, 0, width, height, desktopDC, 0, 0, SRCCOPY | CAPTUREBLT);

        // 获取鼠标光标信息
        CURSORINFO cursorInfo = { sizeof(CURSORINFO) };
        if (GetCursorInfo(&cursorInfo) && (cursorInfo.flags & CURSOR_SHOWING)) {
            ICONINFO iconInfo;
            if (GetIconInfo(cursorInfo.hCursor, &iconInfo)) {
                int x = cursorInfo.ptScreenPos.x - iconInfo.xHotspot;
                int y = cursorInfo.ptScreenPos.y - iconInfo.yHotspot;
                DrawIcon(memoryDC, x, y, cursorInfo.hCursor);
                DeleteObject(iconInfo.hbmMask);
                DeleteObject(iconInfo.hbmColor);
            }
        }

        auto bitmap = std::unique_ptr<Gdiplus::Bitmap>(Gdiplus::Bitmap::FromHBITMAP(hBitmap, nullptr));

        ReleaseDC(GetDesktopWindow(), desktopDC);
        DeleteDC(memoryDC);
        DeleteObject(hBitmap);

        return bitmap;
    }

    static std::vector<char> SaveToBuffer(Gdiplus::Bitmap* bitmap, const wchar_t* format, long quality) {
        if (!bitmap) return {};

        CLSID encoderClsid;
        if (GetEncoderClsid(format, encoderClsid) == -1) return {};

        Gdiplus::EncoderParameters encoderParameters = {};
        if (wcscmp(format, L"image/jpeg") == 0 && quality >= 0 && quality <= 100) {
            encoderParameters.Count = 1;
            encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
            encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
            encoderParameters.Parameter[0].NumberOfValues = 1;
            encoderParameters.Parameter[0].Value = &quality;
        }

        IStream* stream = nullptr;
        if (CreateStreamOnHGlobal(nullptr, TRUE, &stream) != S_OK) return {};

        auto status = bitmap->Save(stream, &encoderClsid, (encoderParameters.Count == 1) ? &encoderParameters : nullptr);
        if (status != Gdiplus::Ok) {
            stream->Release();
            return {};
        }

        LARGE_INTEGER liZero = {};
        ULARGE_INTEGER ulSize;
        stream->Seek(liZero, STREAM_SEEK_END, &ulSize);
        stream->Seek(liZero, STREAM_SEEK_SET, nullptr);

        std::vector<char> buffer(ulSize.LowPart);
        stream->Read(buffer.data(), ulSize.LowPart, nullptr);
        stream->Release();

        return buffer;
    }

private:
    static int GetEncoderClsid(const wchar_t* format, CLSID& clsid) {
        UINT num = 0;
        UINT size = 0;
        Gdiplus::GetImageEncodersSize(&num, &size);
        if (size == 0) return -1;

        auto codecInfo = std::make_unique<Gdiplus::ImageCodecInfo[]>(size);
        if (!codecInfo) return -1;

        Gdiplus::GetImageEncoders(num, size, codecInfo.get());
        for (UINT j = 0; j < num; ++j) {
            if (wcscmp(codecInfo[j].MimeType, format) == 0) {
                clsid = codecInfo[j].Clsid;
                return j;
            }
        }

        return -1;
    }
};


WinScreen::~WinScreen() {
    stop();
}

void WinScreen::start() {
    GdiplusManager::Initialize();
}

void WinScreen::stop() {
    GdiplusManager::Shutdown();
}

void WinScreen::setQuality(int quality) {
    quality_ = quality;
}

std::vector<char> WinScreen::getScreenData() {
    wchar_t format[] = L"image/jpeg";
    auto bitmap = Screenshot::Capture();
    return Screenshot::SaveToBuffer(bitmap.get(), format, quality_);
}
