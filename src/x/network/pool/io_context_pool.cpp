#include "x/network/pool/io_context_pool.h"

namespace x::pool
{

IoContextPool::~IoContextPool()
{
    Stop();
    Wait();
}

void IoContextPool::Start(const size_t pool_size)
{
    StartWithoutRun(pool_size);
    for (auto& ioc : io_context_pool_)
    {
        thread_pool_.emplace_back([&ioc] { ioc.run(); });
    }
}

void IoContextPool::StartWithoutRun(size_t pool_size)
{
    if (pool_size == 0)
    {
        pool_size = std::thread::hardware_concurrency();
    }
    thread_pool_.reserve(pool_size);
    io_context_pool_ = std::vector<boost::asio::io_context>(pool_size);
    work_guard_pool_.reserve(pool_size);
    for (auto& ioc : io_context_pool_)
    {
        work_guard_pool_.emplace_back(boost::asio::make_work_guard(ioc));
    }
}

void IoContextPool::Stop()
{
    for (auto& ioc : io_context_pool_)
    {
        ioc.stop();
    }
}

void IoContextPool::Wait()
{
    for (auto& thread : thread_pool_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

}
