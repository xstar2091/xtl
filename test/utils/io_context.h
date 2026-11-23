#pragma once

#include <thread>
#include <boost/asio.hpp>

namespace x::test
{

class IoContext
{
    boost::asio::io_context ioc_;
    boost::asio::executor_work_guard<decltype(ioc_)::executor_type> ioc_guard_;
    std::thread thread_;

public:
    IoContext();
    ~IoContext();

    [[nodiscard]] boost::asio::io_context& io_context() { return ioc_; }

    void Stop();
    void Wait();
};

}
