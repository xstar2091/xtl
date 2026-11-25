#pragma once

#include <memory>
#include <string>
#include <spdlog/common.h>
#include <spdlog/logger.h>

namespace x
{

struct Log
{
    std::string trace;
    std::string meta;
    const char* module = "";

    struct Logger
    {
        const Log* base = nullptr;
        const char* func = nullptr;
        const char* module = "";
    };

    [[nodiscard]] Log ChildLog() const;
    [[nodiscard]] Logger CreateLogger(const char* func, const char* module_name = nullptr) const
    {
        return Logger{.base = this, .func = func, .module = module_name ? module_name : module};
    }

    static int trace_id_size;
    static std::shared_ptr<spdlog::logger> common_logger;
    // 写入轮转文件
    static void Init(
        const std::string& logger_name,
        const spdlog::filename_t& filename,
        int level,
        size_t max_file_size,
        size_t max_files
    );
    // 写入控制台（终端时为级别着色，重定向时不带颜色码）
    static void Init(const std::string& logger_name, int level);
};

}

#define XLOG(LoggerObj, level, format, ...) \
    x::Log::common_logger->log(\
        spdlog::source_loc{__FILE__, __LINE__, LoggerObj.func},\
        level,\
        "{}|{}|{}| " format,\
        LoggerObj.base->trace,\
        LoggerObj.module,\
        LoggerObj.base->meta,\
        ##__VA_ARGS__\
    )
#define XLOG_FLUSH() x::Log::common_logger->flush()
#define XLOG_INFO(LoggerObj, format, ...)     XLOG(LoggerObj, spdlog::level::info, format, ##__VA_ARGS__)
#define XLOG_WARN(LoggerObj, format, ...)     XLOG(LoggerObj, spdlog::level::warn, format, ##__VA_ARGS__)
#define XLOG_ERROR(LoggerObj, format, ...)    XLOG(LoggerObj, spdlog::level::err, format, ##__VA_ARGS__)
#define XLOG_CRITICAL(LoggerObj, format, ...) XLOG(LoggerObj, spdlog::level::critical, format, ##__VA_ARGS__)
