#include <gtest/gtest.h>

#include <thread>
#include "x/network/http/client.h"
#include "x/network/pool/io_context_pool.h"

namespace
{

const x::http::Response::StringBody& InitClient(x::http::Client& client)
{
    client.host = "www.baidu.com";
    client.port = "80";
    auto& req = client.request.New<x::http::Request::StringBody>();
    req.method(boost::beast::http::verb::get);
    req.target("/");
    req.version(11);
    return client.response.New<x::http::Response::StringBody>();
}

}

TEST(ClientTest, StartFuture)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    const auto& resp = InitClient(client);

    client.timeout = 10000;
    client.StartFuture().get();
    EXPECT_EQ(client.error_code().value(), 0);
    EXPECT_EQ(resp.result(), boost::beast::http::status::ok);
    EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
    EXPECT_FALSE(resp.body().empty());
    EXPECT_EQ(client.in_fly_count(), 0);
}

TEST(ClientTest, StartFutureTimeout)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    InitClient(client);

    client.timeout = 1;
    client.StartFuture().get();
    EXPECT_EQ(client.error_code().value(), 1);
    EXPECT_EQ(client.in_fly_count(), 0);
}

TEST(ClientTest, StartWithRetryFutureOK)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    const auto& resp = InitClient(client);

    client.timeout = 10000;
    client.retry_interval = 10;
    client.max_retry_attempts = 3;
    client.StartWithRetryFuture().get();
    EXPECT_EQ(client.error_code().value(), 0);
    EXPECT_EQ(resp.result(), boost::beast::http::status::ok);
    EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
    EXPECT_FALSE(resp.body().empty());
    EXPECT_TRUE(client.current_attempt() <= client.max_retry_attempts);
    EXPECT_EQ(client.in_fly_count(), 0);
}

TEST(ClientTest, StartWithRetryFutureError)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    InitClient(client);

    client.timeout = 1;
    client.retry_interval = 10;
    client.max_retry_attempts = 3;
    client.StartWithRetryFuture().get();
    EXPECT_EQ(client.error_code().value(), 1);
    EXPECT_EQ(client.current_attempt(), client.max_retry_attempts);
    EXPECT_EQ(client.in_fly_count(), 0);
}

TEST(ClientTest, StartAsync)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    auto& resp = InitClient(client);

    client.timeout = 10000;
    client.StartAsync([&client, &resp, &ioc]
    {
        EXPECT_EQ(client.error_code().value(), 0);
        EXPECT_EQ(resp.result(), boost::beast::http::status::ok);
        EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
        EXPECT_FALSE(resp.body().empty());
        EXPECT_EQ(client.in_fly_count(), 0);
        ioc.Stop();
    });

    ioc.Wait();
}

TEST(ClientTest, StartAsyncTimout)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    InitClient(client);

    client.timeout = 1;
    client.StartAsync([&client, &ioc]
    {
        EXPECT_EQ(client.error_code().value(), 1);
        EXPECT_EQ(client.in_fly_count(), 0);
        ioc.Stop();
    });

    ioc.Wait();
}

TEST(ClientTest, StartWithRetryAsyncOK)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    auto& resp = InitClient(client);

    client.timeout = 10000;
    client.retry_interval = 10;
    client.max_retry_attempts = 3;
    client.StartWithRetryAsync([&client, &resp, &ioc]
    {
        EXPECT_EQ(client.error_code().value(), 0);
        EXPECT_EQ(resp.result(), boost::beast::http::status::ok);
        EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
        EXPECT_FALSE(resp.body().empty());
        EXPECT_TRUE(client.current_attempt() <= client.max_retry_attempts);
        EXPECT_EQ(client.in_fly_count(), 0);
        ioc.Stop();
    });

    ioc.Wait();
}

TEST(ClientTest, StartWithRetryAsyncError)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    InitClient(client);

    client.timeout = 1;
    client.retry_interval = 10;
    client.max_retry_attempts = 3;
    client.StartWithRetryAsync([&client, &ioc]
    {
        EXPECT_EQ(client.error_code().value(), 1);
        EXPECT_EQ(client.current_attempt(), client.max_retry_attempts);
        EXPECT_EQ(client.in_fly_count(), 0);
        ioc.Stop();
    });

    ioc.Wait();
}

TEST(ClientTest, StartFutureOKWithReset)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    const auto* resp = &InitClient(client);

    client.timeout = 10000;
    client.StartFuture().get();
    EXPECT_EQ(client.error_code().value(), 0);
    EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
    EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
    EXPECT_FALSE(resp->body().empty());
    EXPECT_EQ(client.in_fly_count(), 0);

    client.Reset();
    resp = &InitClient(client);
    client.timeout = 10000;
    client.StartFuture().get();
    EXPECT_EQ(client.error_code().value(), 0);
    EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
    EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
    EXPECT_FALSE(resp->body().empty());
    EXPECT_EQ(client.in_fly_count(), 0);
}

TEST(ClientTest, StartFutureErrorWithReset)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    InitClient(client);

    client.timeout = 1;
    client.StartFuture().get();
    EXPECT_EQ(client.error_code().value(), 1);
    EXPECT_EQ(client.in_fly_count(), 0);

    client.Reset();
    const auto* resp = &InitClient(client);
    client.timeout = 10000;
    client.StartFuture().get();
    EXPECT_EQ(client.error_code().value(), 0);
    EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
    EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
    EXPECT_FALSE(resp->body().empty());
    EXPECT_EQ(client.in_fly_count(), 0);
}

TEST(ClientTest, StartWithRetryFutureOKWithReset)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    const auto* resp = &InitClient(client);

    client.timeout = 10000;
    client.retry_interval = 10;
    client.max_retry_attempts = 3;
    client.StartWithRetryFuture().get();
    EXPECT_EQ(client.error_code().value(), 0);
    EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
    EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
    EXPECT_FALSE(resp->body().empty());
    EXPECT_TRUE(client.current_attempt() <= client.max_retry_attempts);
    EXPECT_EQ(client.in_fly_count(), 0);

    client.Reset();
    resp = &InitClient(client);
    client.StartWithRetryFuture().get();
    EXPECT_EQ(client.error_code().value(), 0);
    EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
    EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
    EXPECT_FALSE(resp->body().empty());
    EXPECT_TRUE(client.current_attempt() <= client.max_retry_attempts);
    EXPECT_EQ(client.in_fly_count(), 0);
}

TEST(ClientTest, StartWithRetryFutureErrorWithReset)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    InitClient(client);

    client.timeout = 1;
    client.retry_interval = 10;
    client.max_retry_attempts = 3;
    client.StartWithRetryFuture().get();
    EXPECT_EQ(client.error_code().value(), 1);
    EXPECT_EQ(client.current_attempt(), client.max_retry_attempts);
    EXPECT_EQ(client.in_fly_count(), 0);

    client.Reset();
    const auto* resp = &InitClient(client);
    client.timeout = 10000;
    client.StartWithRetryFuture().get();
    EXPECT_EQ(client.error_code().value(), 0);
    EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
    EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
    EXPECT_FALSE(resp->body().empty());
    EXPECT_TRUE(client.current_attempt() <= client.max_retry_attempts);
    EXPECT_EQ(client.in_fly_count(), 0);
}

TEST(ClientTest, StartAsyncOKWithReset)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    const auto* resp = &InitClient(client);

    client.timeout = 10000;
    client.StartAsync([&client, &resp, &ioc]
    {
        EXPECT_EQ(client.error_code().value(), 0);
        EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
        EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
        EXPECT_FALSE(resp->body().empty());
        EXPECT_EQ(client.in_fly_count(), 0);

        client.Reset();
        resp = &InitClient(client);
        client.timeout = 10000;
        client.StartAsync([&client, &resp, &ioc]
        {
            EXPECT_EQ(client.error_code().value(), 0);
            EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
            EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
            EXPECT_FALSE(resp->body().empty());
            EXPECT_EQ(client.in_fly_count(), 0);
            ioc.Stop();
        });
    });

    ioc.Wait();
}

TEST(ClientTest, StartAsyncErrorWithReset)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    const auto* resp = &InitClient(client);

    client.timeout = 1;
    client.StartAsync([&client, &resp, &ioc]
    {
        EXPECT_EQ(client.error_code().value(), 1);
        EXPECT_EQ(client.in_fly_count(), 0);

        client.Reset();
        resp = &InitClient(client);
        client.timeout = 10000;
        client.StartAsync([&client, &resp, &ioc]
        {
            EXPECT_EQ(client.error_code().value(), 0);
            EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
            EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
            EXPECT_FALSE(resp->body().empty());
            EXPECT_EQ(client.in_fly_count(), 0);
            ioc.Stop();
        });
    });

    ioc.Wait();
}

TEST(ClientTest, StartWithRetryAsyncOKWithReset)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    const auto* resp = &InitClient(client);

    client.timeout = 10000;
    client.retry_interval = 10;
    client.max_retry_attempts = 3;
    client.StartWithRetryAsync([&client, &resp, &ioc]
    {
        EXPECT_EQ(client.error_code().value(), 0);
        EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
        EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
        EXPECT_FALSE(resp->body().empty());
        EXPECT_TRUE(client.current_attempt() <= client.max_retry_attempts);
        EXPECT_EQ(client.in_fly_count(), 0);

        client.Reset();
        resp = &InitClient(client);
        client.timeout = 10000;
        client.StartWithRetryAsync([&client, &resp, &ioc]
        {
            EXPECT_EQ(client.error_code().value(), 0);
            EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
            EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
            EXPECT_FALSE(resp->body().empty());
            EXPECT_EQ(client.in_fly_count(), 0);
            ioc.Stop();
        });
    });

    ioc.Wait();
}

TEST(ClientTest, StartWithRetryAsyncErrorWithReset)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    const auto* resp = &InitClient(client);

    client.timeout = 1;
    client.retry_interval = 10;
    client.max_retry_attempts = 3;
    client.StartWithRetryAsync([&client, &resp, &ioc]
    {
        EXPECT_EQ(client.error_code().value(), 1);
        EXPECT_EQ(client.current_attempt(), client.max_retry_attempts);
        EXPECT_EQ(client.in_fly_count(), 0);

        client.Reset();
        resp = &InitClient(client);
        client.timeout = 10000;
        client.StartWithRetryAsync([&client, &resp, &ioc]
        {
            EXPECT_EQ(client.error_code().value(), 0);
            EXPECT_EQ(resp->result(), boost::beast::http::status::ok);
            EXPECT_EQ(client.response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
            EXPECT_FALSE(resp->body().empty());
            EXPECT_EQ(client.in_fly_count(), 0);
            ioc.Stop();
        });
    });

    ioc.Wait();
}

TEST(ClientTest, StartAsyncWithCancel)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    InitClient(client);

    client.timeout = 10000;
    client.StartAsync([&client, &ioc]
    {
        EXPECT_EQ(client.error_code().value(), boost::beast::errc::operation_canceled);
        ioc.Stop();
    });

    client.Cancel();
    ioc.Wait();
}

TEST(ClientTest, StartWithRetryAsyncWithCancel)
{
    x::pool::IoContextPool ioc;
    ioc.Start(1);
    x::http::Client client(ioc.io_context());
    InitClient(client);

    client.timeout = 10000;
    client.retry_interval = 10;
    client.max_retry_attempts = 3;
    client.StartWithRetryAsync([&client, &ioc]
    {
        EXPECT_EQ(client.error_code().value(), boost::beast::errc::operation_canceled);
        EXPECT_TRUE(client.current_attempt() < client.max_retry_attempts);
        EXPECT_EQ(client.in_fly_count(), 0);
        ioc.Stop();
    });

    client.Cancel();
    ioc.Wait();
}
