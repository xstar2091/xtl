#pragma once

#include <future>
#include <boost/asio/io_context.hpp>
#include <boost/beast.hpp>
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
    template<class CallbackType>
    void StartAsync(CallbackType&& callback);
    template<class CallbackType>
    void StartWithRetryAsync(CallbackType&& callback);

private:
    template<class CallbackType>
    void Resolve(CallbackType&& callback);
    template<class CallbackType>
    void Connect(const boost::asio::ip::tcp::resolver::results_type& results, CallbackType&& callback);
    template<class CallbackType>
    void Write(CallbackType&& callback);
    template<class CallbackType>
    void Read(CallbackType&& callback);
    template<class CallbackType>
    bool HandleError(const boost::system::error_code& ec, CallbackType&& callback);
    template<class CallbackType>
    void HandleFinished(CallbackType&& callback);
};

template <class CallbackType>
void Client::StartAsync(CallbackType&& callback)
{
    in_fly_count_.fetch_add(1);
    boost::asio::dispatch(
        io_context_,
        [this, callback = std::forward<CallbackType>(callback)]() mutable
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

template <class CallbackType>
void Client::StartWithRetryAsync(CallbackType&& callback)
{
    StartAsync(
        [this, callback = std::forward<CallbackType>(callback)]() mutable
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

template <class CallbackType>
void Client::Resolve(CallbackType&& callback)
{
    state_ = RequestState::resolve_started;
    resolver_.async_resolve(
        host, port,
        [this, callback = std::forward<CallbackType>(callback)](
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

template <class CallbackType>
void Client::Connect(const boost::asio::ip::tcp::resolver::results_type& results, CallbackType&& callback)
{
    state_ = RequestState::connect_started;
    stream_.async_connect(
        results,
        [this, callback = std::forward<CallbackType>(callback)](
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

template <class CallbackType>
void Client::Write(CallbackType&& callback)
{
    state_ = RequestState::write_started;
    std::visit([this, callback = std::forward<CallbackType>(callback)]<typename T>([[maybe_unused]] T& arg) mutable
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

template <class CallbackType>
void Client::Read(CallbackType&& callback)
{
    state_ = RequestState::read_started;
    std::visit([this, callback = std::forward<CallbackType>(callback)]<typename T>([[maybe_unused]] T& arg) mutable {
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

template <class CallbackType>
bool Client::HandleError(const boost::system::error_code& ec, CallbackType&& callback)
{
    if (error_code_)
    {
        HandleFinished(callback);
        return true;
    }
    if (ec)
    {
        error_code_ = ec;
        HandleFinished(callback);
        return true;
    }
    return false;
}

template <class CallbackType>
void Client::HandleFinished(CallbackType&& callback)
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
