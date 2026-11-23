#include "utils/io_context.h"

namespace x::test
{

IoContext::IoContext()
    : ioc_guard_(boost::asio::make_work_guard(ioc_))
{
    thread_ = std::thread([this] { ioc_.run(); });
}

IoContext::~IoContext()
{
    if (thread_.joinable())
    {
        ioc_.stop();
        thread_.join();
    }
}

void IoContext::Stop()
{
    ioc_.stop();
}

void IoContext::Wait()
{
    if (thread_.joinable())
    {
        thread_.join();
    }
}

}
