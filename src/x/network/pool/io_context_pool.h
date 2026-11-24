#pragma once

#include <thread>
#include <vector>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>

namespace x::pool
{

class IoContextPool
{
    std::atomic<size_t> index_{0};
    std::vector<std::thread> thread_pool_;
    std::vector<boost::asio::io_context> io_context_pool_;
    std::vector<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_guard_pool_;

public:
    IoContextPool() = default;
    IoContextPool(IoContextPool&&) = delete;
    IoContextPool(const IoContextPool&) = delete;
    IoContextPool& operator=(IoContextPool&&) = delete;
    IoContextPool& operator=(const IoContextPool&) = delete;
    ~IoContextPool();

    boost::asio::io_context& io_context(const size_t index) { return io_context_pool_[index]; }
    boost::asio::io_context& io_context() { return io_context_pool_[index_.fetch_add(1) % io_context_pool_.size()]; }

    void Start(size_t pool_size);
    void StartWithoutRun(size_t pool_size);
    void Stop();
    void Wait();
};

}
