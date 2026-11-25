#include "x/log.h"

#include <utility>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "x/uuid.h"

namespace x
{

int Log::trace_id_size = 32;
std::shared_ptr<spdlog::logger> Log::common_logger;

namespace
{

void Configure(std::shared_ptr<spdlog::logger> logger, int level)
{
    if (level < spdlog::level::trace || level >= spdlog::level::off)
    {
        level = spdlog::level::info;
    }
    // spdlog::flush_every(std::chrono::seconds(1));
    logger->flush_on(spdlog::level::info);
    logger->set_level(static_cast<spdlog::level::level_enum>(level));
    // %^...%$ 只影响控制台着色，文件输出不会出现颜色码
    logger->set_pattern("%Y-%m-%d %H:%M:%S.%e|%^%l%$|%P|%t|%s:%#|%!|%v");
    Log::common_logger = std::move(logger);
}

}

void Log::Init(
    const std::string& logger_name,
    const spdlog::filename_t& filename,
    const int level,
    const size_t max_file_size,
    const size_t max_files
)
{
    Configure(spdlog::rotating_logger_mt(logger_name, filename, max_file_size, max_files), level);
}

void Log::Init(const std::string& logger_name, const int level)
{
    Configure(spdlog::stdout_color_mt(logger_name), level);
}

Log Log::ChildLog() const
{
    Log log{.module = module};
    log.meta = meta;
    log.trace.reserve(trace.size() + 1 + trace_id_size);
    log.trace.append(trace).append(":").append(Uuid{.remote_hyphen = true}.Random());
    return log;
}

}
