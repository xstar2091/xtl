#include "x/network/http/client.h"

namespace x::http
{

Client::Client(boost::asio::io_context& ioc)
    : Client(ioc, 0)
{
}

Client::Client(boost::asio::io_context& ioc, const int index)
    : state_(RequestState::finished)
    , keep_alive_(false)
    , current_attempt_(1)
    , in_fly_count_(0)
    , index_(index)
    , io_context_(ioc)
    , retry_timer_(ioc)
    , resolver_(ioc)
    , stream_(ioc)
    , timeout(1000)
    , retry_interval(0)
    , max_retry_attempts(1)
    , host("localhost")
    , port("80")
{
}

void Client::Cancel()
{
    boost::asio::dispatch(
        io_context_,
        [this]
        {
            if (state_ != RequestState::finished)
            {
                error_code_ = boost::beast::errc::make_error_code(boost::beast::errc::operation_canceled);
                stream_.cancel();
            }
        }
    );
}

void Client::Reset()
{
    if (state_ != RequestState::finished) return;
    current_attempt_ = 1;
    buffer_.clear();
    error_code_ = boost::system::error_code();
    remote_endpoint_ = boost::asio::ip::tcp::endpoint();
    request.Reset();
    response.Reset();
}

std::future<void> Client::StartFuture()
{
    std::promise<void> promise;
    auto future = promise.get_future();
    StartAsync([promise = std::move(promise)]() mutable { promise.set_value(); });
    return future;
}

std::future<void> Client::StartWithRetryFuture()
{
    std::promise<void> promise;
    auto future = promise.get_future();
    StartWithRetryAsync([promise = std::move(promise)]() mutable { promise.set_value(); });
    return future;
}

}
