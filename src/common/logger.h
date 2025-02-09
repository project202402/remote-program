/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include <cstdio>
#include <cassert>
#include <string>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <array>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <utility>

#if defined(__linux__)
#include <unistd.h>
#include <sys/syscall.h>
#endif

    class LogLevel
	{
	 public:

		enum class Level {
			debug,
			info,
			warn,
			error,
			fatal
		};

		static std::string toString(Level level) {
			static std::map<LogLevel::Level, std::string> levelStrings = {
				{LogLevel::Level::info,		"INFO"	},
				{LogLevel::Level::debug,	"DEBUG"	},
				{LogLevel::Level::warn,		"WARN"	},
				{LogLevel::Level::error,	"ERROR"	},
				{LogLevel::Level::fatal,	"FATAL"	},
			};

			if (levelStrings.find(level) != levelStrings.end()) {
				return levelStrings[level];
			}

			return "UNKNOW";
		}
	};

	class LogEvent
	{
	public:
		using Ptr = std::unique_ptr<LogEvent>;

		LogEvent(const std::string& name, LogLevel::Level level, const char* file, uint32_t line,
			std::thread::id threadId, uint64_t time) :
			m_name(name), m_level(level), m_file(file),
			m_line(line), m_threadId(threadId), m_time(time) {}

		LogLevel::Level level() {
			return m_level;
		}

		std::stringstream& steam() {
			return m_ss;
		}

		template <class... Args>
		void format(const char* fmt, const Args &...args) {
			const auto bufSize = 2048;
			std::array<char, bufSize> buf{};

			auto sn = snprintf(buf.data(), buf.size() - 1, fmt, args...);
			if (sn <= 0) {
				std::string info = "dropped log messages because snprintf error: ";
				info.append(std::to_string(sn));
				m_ss << info;
				return;
			}

			auto n = static_cast<size_t>(sn);
			if (n >= buf.size() - 1) {
				std::vector<char> glowableBuffer(buf.size());

				while (n >= glowableBuffer.size() - 1) {
					glowableBuffer.resize(glowableBuffer.size() * 2);
					n = static_cast<size_t>(
						snprintf(&glowableBuffer[0], glowableBuffer.size() - 1, fmt, args...));
				}
				m_ss << std::string(&glowableBuffer[0], n);
				return;
			}
			m_ss << std::string(buf.data(), n);
		}

		virtual std::string toString() {

			std::stringstream ss;
			ss << timeToString("%F %T") << " ";
			ss << m_name << " ";

#if defined(_WIN32)
			ss << m_threadId << " ";
#else
			ss << syscall(SYS_gettid) << " ";
#endif
			ss << "[" << LogLevel::toString(m_level) << "]" << " ";
#ifdef DEBUG
			ss << m_file << ":" << m_line << " ";
#endif
			ss << m_ss.str() << "\n";

			std::string buffer = ss.str();
			return std::move(buffer);
		}

	 protected:

		std::string timeToString(const std::string& format) {

			std::stringstream ss;
			struct tm tm;
#if defined(_WIN32)
			localtime_s(&tm, &m_time);
#else 
			localtime_r(&m_time, &tm);
#endif
			ss << std::put_time(&tm, format.c_str());
			return ss.str();
		}

	 private:

		std::string			m_name = "";
		LogLevel::Level		m_level = LogLevel::Level::debug;
		const char*			m_file = nullptr;
		uint32_t			m_line = -1;
		std::thread::id		m_threadId;
		time_t				m_time = 0;
		std::stringstream	m_ss;
	};


	class LogAppender
	{
	 public:

		using Ptr = std::shared_ptr<LogAppender>;

		virtual ~LogAppender() = default;

		virtual void log(LogEvent* event) {
			const std::string& buffer = event->toString();
			output(buffer.c_str(), buffer.length());
		}

		virtual void output(const char* buffer, size_t len) = 0;
		virtual void flush() = 0;

	};

	//muduo

	template<class Appender>
	class AsyncLogAppender : public Appender
	{
	 public:
		template<class ... Args>
		AsyncLogAppender(size_t interval = 3, Args&& ... args) : Appender(std::forward<Args>(args)...),
			m_running(false), m_thread([this]() noexcept { threadFunc(); }), m_interval(interval) {}

		virtual ~AsyncLogAppender() {
			if (m_running) {
				m_running = false;
				m_cond.notify_all();
				m_thread.join();
			}
		}

	 private:

		void output(const char* buffer, size_t len) override {

			std::unique_lock<std::mutex> lock(m_mutex);
			if (m_current->avail() > len) {
				m_current->append(buffer, len);
				return;
			}

			m_buffers.push_back(std::move(m_current));
			if (m_next) {
				m_current = std::move(m_next);
			}
			else {
				m_current = std::make_unique<Buffer>();
			}
			m_current->append(buffer, len);
			m_cond.notify_all();
		}
		
		void flush() override {

			m_cond.notify_all();
		}
		void threadFunc() {

			m_running = true;	//fix
			BufferPtr  newBuffer1 = std::make_unique<Buffer>();
			BufferPtr  newBuffer2 = std::make_unique<Buffer>();
			newBuffer1->clear();
			newBuffer2->clear();
			BufferVector buffersToWrite;
			buffersToWrite.reserve(16);
			while (m_running) {

				assert(newBuffer1 && newBuffer1->size() == 0);
				assert(newBuffer2 && newBuffer2->size() == 0);
				assert(buffersToWrite.empty());

				{
					std::unique_lock<std::mutex> lock(m_mutex);
					if (m_buffers.empty()) {
						m_cond.wait_for(lock, std::chrono::seconds(m_interval));
					}
					m_buffers.push_back(std::move(m_current));
					m_current = std::move(newBuffer1);
					buffersToWrite.swap(m_buffers);
					if (!m_next) {
						m_next = std::move(newBuffer2);
					}
				}

				assert(!buffersToWrite.empty());

				if (buffersToWrite.size() > 25) {
					std::string info = "dropped log messages because larger buffers\n";
					Appender::output(info.data(), info.size());
					buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
				}

				for (const auto& buffer : buffersToWrite) {
					Appender::output(buffer->data(), buffer->size());
				}

				if (buffersToWrite.size() > 2) {
					buffersToWrite.resize(2);
				}

				if (!newBuffer1) {
					assert(!buffersToWrite.empty());
					newBuffer1 = std::move(buffersToWrite.back());
					buffersToWrite.pop_back();
					newBuffer1->clear();
				}

				if (!newBuffer2) {
					assert(!buffersToWrite.empty());
					newBuffer2 = std::move(buffersToWrite.back());
					buffersToWrite.pop_back();
					newBuffer2->clear();
				}

				buffersToWrite.clear();
				Appender::flush();
			}

			Appender::flush();
		}

		template<size_t BUFFSIZE>
		class FixBuffer {
		 public:
			void append(const char* buffer, size_t len) {
				std::copy(buffer, buffer + len, m_buffer.begin() + m_index);
				m_index += len;
			}

			size_t avail() {
				assert(BUFFSIZE > m_index);
				return BUFFSIZE - m_index;
			}

			const char* data() {
				return m_buffer.data();
			}

			size_t size() {
				return m_index;
			}

			void clear() {
				m_buffer.fill(0);
				m_index = 0;
			}

		 private:
			std::array<char, BUFFSIZE>	m_buffer{};
			size_t						m_index = 0;
		};

		static constexpr size_t BufferSize = 4 * 1024;
		using Buffer = FixBuffer<BufferSize>;
		using BufferPtr = std::unique_ptr<Buffer>;
		using BufferVector = std::vector<BufferPtr>;


		std::atomic<bool>			m_running;
		BufferPtr					m_current = std::make_unique<Buffer>();
		BufferPtr					m_next = std::make_unique<Buffer>();
		BufferVector				m_buffers{};

		std::thread					m_thread;
		size_t						m_interval;
		std::mutex					m_mutex;
		std::condition_variable		m_cond;
	};

	class StdOutLogAppender : public LogAppender
	{
	  public:
		void output(const char* buffer, size_t len) override {

			std::lock_guard<std::mutex> lock(m_mutex);
			fwrite(buffer, 1, len, stdout);
			flush();
		}

		void flush() override {

			fflush(stdout);
		}

	  private:

		std::mutex	m_mutex;
	};

	class Logger
	{
	 public:

		using Ptr = std::shared_ptr<Logger>;

		Logger(const std::string& name, LogLevel::Level level, LogAppender::Ptr appender) :
			m_name(name), m_level(level){
			m_appenders.emplace_back(std::move(appender));
		}


		void setName(const std::string& name) {
			m_name = name;
		}

		std::string& getName() {
			return m_name;
		}

		void setLevel(LogLevel::Level level) {
			m_level = level;
		}

		LogLevel::Level getLevel() {
			return m_level;
		}

		void addAppender(LogAppender::Ptr appender) {
			m_appenders.emplace_back(std::move(appender));
		}

		void log(LogEvent* event) {
			auto level = event->level();
			if (level >= m_level) {
				for (auto& it : m_appenders) {
					it->log(event);
				}
			}

			if (level == LogLevel::Level::fatal) {
				for (auto& it : m_appenders) {
					it->flush();
				}
				std::abort();
			}
		}

	  private:
		std::string					m_name;
		LogLevel::Level				m_level = LogLevel::Level::debug;
		std::list<LogAppender::Ptr>	m_appenders;
	};


	class LogEventGuard {
	 public:
		LogEventGuard(Logger* logger, LogEvent&& event) :
			m_logger(logger), m_event(std::move(event)) {
		}

		~LogEventGuard() {
			m_logger->log(&m_event);
		}

		std::stringstream& steam() {
			return m_event.steam();
		}

		template <typename... Args>
		void format(const char* fmt, const Args &...args) {
			m_event.format(fmt, args...);
		}

	 private:

		Logger*			m_logger;
		LogEvent	    m_event;
	};


/********************************************************************************/

struct AsyncLogger {
	static Logger* singleton() {
		static Logger logger("default", LogLevel::Level::debug,
			std::make_unique<AsyncLogAppender<StdOutLogAppender>>(3));
		return &logger;
	}
};

struct SyncLogger {
	static Logger* singleton() {
		static Logger logger("default", LogLevel::Level::debug,
			std::make_unique<StdOutLogAppender>());
		return &logger;
	}
};

using DefaultLogger = SyncLogger;

#define LOG_LEVEL(logger, level)\
    LogEventGuard( logger, \
		LogEvent(logger->getName(), level, __FILE__, __LINE__, \
				std::this_thread::get_id(), time(nullptr))).steam()

#define LOG_DEBUG(logger)	LOG_LEVEL(logger, LogLevel::Level::debug)
#define LOG_INFO(logger)	LOG_LEVEL(logger, LogLevel::Level::info)
#define LOG_WARN(logger)	LOG_LEVEL(logger, LogLevel::Level::warn)
#define LOG_ERROR(logger)	LOG_LEVEL(logger, LogLevel::Level::error)
#define LOG_FATAL(logger)	LOG_LEVEL(logger, LogLevel::Level::fatal)

#define LOG_FMT_LEVEL(logger, level, fmt, ...) \
    LogEventGuard( logger, \
		LogEvent(logger->getName(), level, __FILE__, __LINE__, \
				std::this_thread::get_id(), time(nullptr))).format(fmt, ##__VA_ARGS__)

#define LOG_DEBUG_FMT(logger, fmt, ...)	LOG_FMT_LEVEL(logger, LogLevel::Level::debug, fmt, ##__VA_ARGS__)
#define LOG_INFO_FMT(logger, fmt, ...)  LOG_FMT_LEVEL(logger, LogLevel::Level::info, fmt, ##__VA_ARGS__)
#define LOG_WARN_FMT(logger, fmt, ...)  LOG_FMT_LEVEL(logger, LogLevel::Level::warn, fmt, ##__VA_ARGS__)
#define LOG_ERROR_FMT(logger, fmt, ...) LOG_FMT_LEVEL(logger, LogLevel::Level::error, fmt, ##__VA_ARGS__)
#define LOG_FATAL_FMT(logger, fmt, ...) LOG_FMT_LEVEL(logger, LogLevel::Level::fatal, fmt, ##__VA_ARGS__)

#define ZLOG_DEBUG		LOG_DEBUG(DefaultLogger::singleton())
#define ZLOG_INFO		LOG_INFO(DefaultLogger::singleton())
#define ZLOG_WARN		LOG_WARN(DefaultLogger::singleton())
#define ZLOG_ERROR		LOG_ERROR(DefaultLogger::singleton())
#define ZLOG_FATAL		LOG_FATAL(DefaultLogger::singleton())

#define ZLOG_DEBUG_FMT(fmt, ...)	LOG_DEBUG_FMT(DefaultLogger::singleton(), fmt, ##__VA_ARGS__)
#define ZLOG_INFO_FMT(fmt, ...)		LOG_INFO_FMT(DefaultLogger::singleton(), fmt, ##__VA_ARGS__)
#define ZLOG_WARN_FMT(fmt, ...)		LOG_WARN_FMT(DefaultLogger::singleton(), fmt, ##__VA_ARGS__)
#define ZLOG_ERROR_FMT(fmt, ...)	LOG_ERROR_FMT(DefaultLogger::singleton(), fmt, ##__VA_ARGS__)
#define ZLOG_FATAL_FMT(fmt, ...)	LOG_FATAL_FMT(DefaultLogger::singleton(), fmt, ##__VA_ARGS__)


/********************************************************************************/
