// From https://github.com/leiradel/ImGuiAl
/*
The MIT License (MIT)

Copyright (c) 2019 Andre Leiradella

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "common/logger.h"
#include <imgui.h>
#include <stdarg.h>

#include <functional>

class Fifo {
   public:
    Fifo(void* const buffer, size_t const size);

    void reset();
    size_t size() const { return _size; }
    size_t occupied() const { return _size - _available; }
    size_t available() const { return _available; }

    void read(void* const data, size_t const count);
    void peek(size_t const pos, void* const data, size_t const count) const;
    void write(void const* const data, size_t const count);
    void skip(size_t const count);

   protected:
    void* _buffer;
    size_t const _size;
    size_t _available;
    size_t _first;
    size_t _last;
};

class Crt : public LogAppender {
   public:
    struct CGA {
        enum : ImU32 {
            Black         = IM_COL32(0x00, 0x00, 0x00, 0xff),
            Blue          = IM_COL32(0x00, 0x00, 0xaa, 0xff),
            Green         = IM_COL32(0x00, 0xaa, 0x00, 0xff),
            Cyan          = IM_COL32(0x00, 0xaa, 0xaa, 0xff),
            Red           = IM_COL32(0xaa, 0x00, 0x00, 0xff),
            Magenta       = IM_COL32(0xaa, 0x00, 0xaa, 0xff),
            Brown         = IM_COL32(0xaa, 0x55, 0x00, 0xff),
            White         = IM_COL32(0xaa, 0xaa, 0xaa, 0xff),
            Gray          = IM_COL32(0x55, 0x55, 0x55, 0xff),
            BrightBlue    = IM_COL32(0x55, 0x55, 0xff, 0xff),
            BrightGreen   = IM_COL32(0x55, 0xff, 0x55, 0xff),
            BrightCyan    = IM_COL32(0x55, 0xff, 0xff, 0xff),
            BrightRed     = IM_COL32(0xff, 0x55, 0x55, 0xff),
            BrightMagenta = IM_COL32(0xff, 0x55, 0xff, 0xff),
            Yellow        = IM_COL32(0xff, 0xff, 0x55, 0xff),
            BrightWhite   = IM_COL32(0xff, 0xff, 0xff, 0xff)
        };
    };

    struct Info {
        ImU32 foregroundColor;
        unsigned length;
        unsigned metaData;
    };

    Crt(void* const buffer, size_t const size);

    void output(const char* buffer, size_t len) override;
    void flush() {};

    void scrollToBottom();
    void clear();

    void iterate(const std::function<bool(Info const& header, char const* const line)>& iterator) const;
    void draw(ImVec2 const& size = ImVec2(0.0f, 0.0f));

   protected:
    void draw(ImVec2 const& size, const std::function<bool(Info const& header, char const* const line)>& filter);

    Fifo _fifo;
    bool _scrollToBottom;
    bool autoScrollToBotttom = false;
};

class LoggerWindow : public Crt {
   public:
    typedef Crt::Info Info;

    static std::shared_ptr<LoggerWindow> singleton() {
        static constexpr size_t gMaxBufferSize = 600000;
        static char gLogBuffer_[gMaxBufferSize];
        static std::shared_ptr<LoggerWindow> loggerWindow = std::make_shared<LoggerWindow>(gLogBuffer_, gMaxBufferSize);
        return loggerWindow;
    }

    LoggerWindow(void* const buffer, size_t const buffer_size);

    void log(LogEvent *event) override;
    void clear() { Crt::clear(); }
    void iterate(const std::function<bool(Info const& header, char const* const line)>& iterator) const { Crt::iterate(iterator); }
    void scrollToBottom() { Crt::scrollToBottom(); }

    int draw(ImVec2 const& size = ImVec2(0.0f, 0.0f));

    void setColor(LogLevel::Level const level, ImU32 const color);
    void setLabel(LogLevel::Level const level, char const* const label);
    void setCumulativeLabel(char const* const label);
    void setFilterLabel(char const* const label);
    void setFilterHeaderLabel(char const* const label);
    void setActions(char const* actions[]);
    void setColorsAutoFromWindowBg();

   protected:
    ImU32 _debugTextColor;
    ImU32 _debugButtonColor;
    ImU32 _debugButtonHoveredColor;

    ImU32 _infoTextColor;
    ImU32 _infoButtonColor;
    ImU32 _infoButtonHoveredColor;

    ImU32 _warningTextColor;
    ImU32 _warningButtonColor;
    ImU32 _warningButtonHoveredColor;

    ImU32 _errorTextColor;
    ImU32 _errorButtonColor;
    ImU32 _errorButtonHoveredColor;
    bool  _useAutomaticColors = true;

    char const* _debugLabel;
    char const* _infoLabel;
    char const* _warningLabel;
    char const* _errorLabel;

    char const* _cumulativeLabel;
    char const* _filterLabel;
    char const* _filterHeaderLabel;

    bool _showFilters;
    char const* const* _actions;

    LogLevel::Level _level;
    bool _cumulative;
    ImGuiTextFilter _filter;
};



/********************************************************************************/

struct ImguiLogger {
	static Logger* singleton() {
	    static std::shared_ptr<LogAppender> pointer = std::static_pointer_cast<LogAppender>(LoggerWindow::singleton());
		static Logger logger("default", LogLevel::Level::debug, pointer);
		return &logger;
	}
};

#define IMGUI_DEBUG		LOG_DEBUG(ImguiLogger::singleton())
#define IMGUI_INFO		LOG_INFO(ImguiLogger::singleton())
#define IMGUI_WARN		LOG_WARN(ImguiLogger::singleton())
#define IMGUI_ERROR		LOG_ERROR(ImguiLogger::singleton())
#define IMGUI_FATAL		LOG_FATAL(ImguiLogger::singleton())

#define IMGUI_DEBUG_FMT(fmt, ...)	LOG_DEBUG_FMT(ImguiLogger::singleton(), fmt, ##__VA_ARGS__)
#define IMGUI_INFO_FMT(fmt, ...)	LOG_INFO_FMT(ImguiLogger::singleton(), fmt, ##__VA_ARGS__)
#define IMGUI_WARN_FMT(fmt, ...)	LOG_WARN_FMT(ImguiLogger::singleton(), fmt, ##__VA_ARGS__)
#define IMGUI_ERROR_FMT(fmt, ...)	LOG_ERROR_FMT(ImguiLogger::singleton(), fmt, ##__VA_ARGS__)
#define IMGUI_FATAL_FMT(fmt, ...)	LOG_FATAL_FMT(ImguiLogger::singleton(), fmt, ##__VA_ARGS__)

/********************************************************************************/