#include <iostream>
#include <thread>
#include "x/network/http/client.h"

int main()
{
    std::cout << "Hello, World!" << std::endl;
    boost::asio::io_context ioc;
    auto work_guard = boost::asio::make_work_guard(ioc);
    std::thread thrd([&ioc] { ioc.run(); });
    x::http::Client client(ioc);
    client.host = "www.baidu.com";
    auto& req = client.request.New<x::http::Request::EmptyBody>();
    req.method(boost::beast::http::verb::get);
    req.target("/");
    req.version(11);
    auto& resp = client.response.New<x::http::Response::StringBody>();
    auto future = client.StartFuture();
    future.get();
    std::cout << resp.body() << std::endl;
    std::cout << client.response.StatusCode() << std::endl;
    std::cout << client.error_code().value() << ": " << client.error_code().message() << std::endl;
    std::cout << "current attempt: " << client.current_attempt() << std::endl;
    std::cout << "in fly count:    " << client.in_fly_count() << std::endl;
    thrd.join();
    return 0;
}
