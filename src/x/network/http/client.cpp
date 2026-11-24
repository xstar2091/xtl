#include "x/network/http/client.h"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>

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

void Client::StartAsync(std::move_only_function<void()> callback)
{
    in_fly_count_.fetch_add(1);
    boost::asio::dispatch(
        io_context_,
        [this, callback = std::move(callback)]() mutable
        {
            if (error_code_ || state_ != RequestState::finished)
            {
                HandleFinished(std::move(callback));
            }
            else
            {
                if (timeout > 0) stream_.expires_after(std::chrono::milliseconds(timeout));
                else stream_.expires_never();
                if (stream_.socket().is_open()) Write(std::move(callback));
                else Resolve(std::move(callback));
            }
        }
    );
}

void Client::StartWithRetryAsync(std::move_only_function<void()> callback)
{
    StartAsync(
        [this, callback = std::move(callback)]() mutable
        {
            if (error_code_ == boost::beast::errc::operation_canceled)
            {
                callback();
                return;
            }
            if (const int http_status_code = response.StatusCode();
                current_attempt_ >= max_retry_attempts ||
                (!error_code_.failed() && http_status_code >= 200 && http_status_code <= 299)
            )
            {
                callback();
                return;
            }
            buffer_.clear();
            error_code_.clear();
            remote_endpoint_ = boost::asio::ip::tcp::endpoint();
            if (retry_interval > 0)
            {
                retry_timer_.expires_after(std::chrono::milliseconds(retry_interval));
                retry_timer_.async_wait(
                    [this, callback = std::move(callback)](const boost::system::error_code& ec) mutable
                    {
                        if (ec)
                        {
                            error_code_ = ec;
                            callback();
                        }
                        else
                        {
                            ++current_attempt_;
                            StartWithRetryAsync(std::move(callback));
                        }
                    }
                );
            }
            else
            {
                ++current_attempt_;
                StartWithRetryAsync(std::move(callback));
            }
        }
    );
}

void Client::Resolve(std::move_only_function<void()> callback)
{
    state_ = RequestState::resolve_started;
    resolver_.async_resolve(
        host, port,
        [this, callback = std::move(callback)](
            const boost::system::error_code& ec,
            const boost::asio::ip::tcp::resolver::results_type& results
        ) mutable
        {
            state_ = RequestState::resolve_finished;
            if (!HandleError(ec, callback))
            {
                Connect(results, std::move(callback));
            }
        }
    );
}

void Client::Connect(const boost::asio::ip::tcp::resolver::results_type& results, std::move_only_function<void()> callback)
{
    state_ = RequestState::connect_started;
    stream_.async_connect(
        results,
        [this, callback = std::move(callback)](
            const boost::system::error_code& ec,
            const boost::asio::ip::tcp::endpoint& remote_endpoint
        ) mutable
        {
            state_ = RequestState::connect_finished;
            if (!HandleError(ec, callback))
            {
                remote_endpoint_ = remote_endpoint;
                Write(std::move(callback));
            }
        }
    );
}

void Client::Write(std::move_only_function<void()> callback)
{
    state_ = RequestState::write_started;
    std::visit([this, callback = std::move(callback)]<typename T>([[maybe_unused]] T& arg) mutable
    {
        using ActuallyRequestType = std::decay_t<T>;
        if constexpr (std::is_same_v<ActuallyRequestType, std::monostate>)
        {
            error_code_ = boost::beast::errc::make_error_code(boost::beast::errc::invalid_argument);
            HandleFinished(std::move(callback));
        }
        else
        {
            auto& req = request.Get<ActuallyRequestType>();
            if (req.count(boost::beast::http::field::host) == 0)
            {
                if (port.empty()) req.set(boost::beast::http::field::host, host);
                else req.set(boost::beast::http::field::host, host + ":" + port);
            }
            boost::beast::http::async_write(
                stream_, req,
                [this, callback = std::move(callback)](const boost::beast::error_code& ec, size_t) mutable
                {
                    state_ = RequestState::write_finished;
                    if (!HandleError(ec, callback))
                    {
                        Read(std::move(callback));
                    }
                }
            );
        }
    }, request.Raw());
}

void Client::Read(std::move_only_function<void()> callback)
{
    state_ = RequestState::read_started;
    std::visit([this, callback = std::move(callback)]<typename T>([[maybe_unused]] T& arg) mutable {
        using ActuallyResponseType = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<ActuallyResponseType, std::monostate>)
        {
            error_code_ = boost::beast::errc::make_error_code(boost::beast::errc::invalid_argument);
            HandleFinished(std::move(callback));
        }
        else
        {
            boost::beast::http::async_read(
                stream_, buffer_, response.Get<ActuallyResponseType>(),
                [this, callback = std::move(callback)](const boost::beast::error_code& ec, size_t) mutable
                {
                    state_ = RequestState::read_finished;
                    if (!HandleError(ec, callback))
                    {
                        HandleFinished(std::move(callback));
                    }
                }
            );
        }
    }, response.Raw());
}

bool Client::HandleError(const boost::system::error_code& ec, std::move_only_function<void()>& callback)
{
    if (error_code_)
    {
        HandleFinished(std::move(callback));
        return true;
    }
    if (ec)
    {
        error_code_ = ec;
        HandleFinished(std::move(callback));
        return true;
    }
    return false;
}

void Client::HandleFinished(std::move_only_function<void()> callback)
{
    if (in_fly_count_.fetch_sub(1) != 1)
    {
        const auto ec = error_code_;
        error_code_ = boost::beast::errc::make_error_code(boost::beast::errc::operation_in_progress);
        callback();
        error_code_ = ec;
        return;
    }

    state_ = RequestState::finished;
    stream_.expires_never();
    if (stream_.socket().is_open())
    {
        if (error_code_ || !keep_alive_)
        {
            boost::system::error_code ec;
            ec = stream_.socket().shutdown(boost::asio::socket_base::shutdown_both, ec);
            stream_.close();
        }
    }

    callback();
}

}
