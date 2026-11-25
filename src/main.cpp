#include "x/log.h"
#include "x/uuid.h"

int main()
{
    x::Log::Init("default", spdlog::level::info);
    const x::Log log{.trace = x::Uuid{.with_hyphen = false}.Random(), .meta = "car001:id001:imei001", .module = "sim" };
    const x::Log::Logger logger = log.CreateLogger("MyMain");
    XLOG_INFO(logger, "log for info");
    XLOG_WARN(logger, "log for warn");
    XLOG_ERROR(logger, "log for error");
    XLOG_CRITICAL(logger, "log for critical");

    const auto log2 = log.ChildLog();
    const auto logger2 = log2.CreateLogger("MyMain2");
    XLOG_INFO(logger2, "log for info");
    XLOG_WARN(logger2, "log for warn");
    XLOG_ERROR(logger2, "log for error");
    XLOG_CRITICAL(logger2, "log for critical");
    return 0;
}
