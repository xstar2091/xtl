#pragma once

#include <functional>
#include <future>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include "x/network/http/request.h"
#include "x/network/http/response.h"

namespace x::http
{

class Client
{
    enum class RequestState : std::uint8_t
    {
        finished,
        resolve_started,
        resolve_finished,
        connect_started,
        connect_finished,
        write_started,
        write_finished,
        read_started,
        read_finished,
    } state_;
    bool keep_alive_;
    int current_attempt_;
    std::atomic<int> in_fly_count_;
    int64_t index_;
    boost::asio::io_context& io_context_;
    boost::asio::steady_timer retry_timer_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::tcp_stream stream_;
    boost::beast::flat_buffer buffer_;
    boost::system::error_code error_code_;
    boost::asio::ip::tcp::endpoint remote_endpoint_;

public:
    int timeout;
    int retry_interval;
    int max_retry_attempts;
    std::string host;
    std::string port;
    Request request;
    Response response;

    explicit Client(boost::asio::io_context& ioc);
    Client(boost::asio::io_context& ioc, int index);
    Client(Client&&) = delete;
    Client(const Client&) = delete;
    Client& operator=(Client&&) = delete;
    Client& operator=(const Client&) = delete;

    [[nodiscard]] int current_attempt() const { return current_attempt_; }
    [[nodiscard]] const boost::system::error_code& error_code() const { return error_code_; }
    [[nodiscard]] const boost::asio::io_context& io_context() const { return io_context_; }
    [[nodiscard]] boost::asio::io_context& io_context() { return io_context_; }
    [[nodiscard]] int in_fly_count() const { return in_fly_count_.load(); }
    [[nodiscard]] int64_t index() const { return index_; }

    void Cancel();
    void Reset();
    std::future<void> StartFuture();
    std::future<void> StartWithRetryFuture();
    void StartAsync(std::move_only_function<void()> callback);
    void StartWithRetryAsync(std::move_only_function<void()> callback);

private:
    void Resolve(std::move_only_function<void()> callback);
    void Connect(const boost::asio::ip::tcp::resolver::results_type& results, std::move_only_function<void()> callback);
    void Write(std::move_only_function<void()> callback);
    void Read(std::move_only_function<void()> callback);
    bool HandleError(const boost::system::error_code& ec, std::move_only_function<void()>& callback);
    void HandleFinished(std::move_only_function<void()> callback);
};

}
