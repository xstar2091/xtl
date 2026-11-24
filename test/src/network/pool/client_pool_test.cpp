#include <latch>
#include <gtest/gtest.h>

#include "x/network/http/client.h"
#include "x/network/pool/client_pool.h"
#include "x/network/pool/io_context_pool.h"

TEST(ClientPoolTest, ReadyCountFuture)
{
    x::pool::IoContextPool io_context_pool;
    io_context_pool.Start(1);
    x::pool::ClientPool<x::http::Client> pool(io_context_pool.io_context());
    pool.SetClientCreateFactory([](boost::asio::io_context& ioc) { return new x::http::Client(ioc); });

    EXPECT_EQ(pool.ReadyCount().get(), 0);
    auto* c1 = pool.GetClient().get();
    auto* c2 = pool.GetClient().get();
    auto* c3 = pool.GetClient().get();
    EXPECT_EQ(pool.ReadyCount().get(), 0);

    pool.Release(c1).get();
    EXPECT_EQ(pool.ReadyCount().get(), 1);
    pool.Release(c2).get();
    EXPECT_EQ(pool.ReadyCount().get(), 2);
    pool.Release(c3).get();
    EXPECT_EQ(pool.ReadyCount().get(), 3);

    c1 = pool.GetClient().get();
    EXPECT_EQ(pool.ReadyCount().get(), 2);
    pool.Release(c1).get();
    EXPECT_EQ(pool.ReadyCount().get(), 3);

    c1 = pool.GetClient().get();
    EXPECT_EQ(pool.ReadyCount().get(), 2);
    c2 = pool.GetClient().get();
    EXPECT_EQ(pool.ReadyCount().get(), 1);
    pool.Release(c2).get();
    EXPECT_EQ(pool.ReadyCount().get(), 2);
    pool.Release(c1).get();
    EXPECT_EQ(pool.ReadyCount().get(), 3);

    c1 = pool.GetClient().get();
    EXPECT_EQ(pool.ReadyCount().get(), 2);
    c2 = pool.GetClient().get();
    EXPECT_EQ(pool.ReadyCount().get(), 1);
    c3 = pool.GetClient().get();
    EXPECT_EQ(pool.ReadyCount().get(), 0);
    pool.Release(c3).get();
    EXPECT_EQ(pool.ReadyCount().get(), 1);
    pool.Release(c2).get();
    EXPECT_EQ(pool.ReadyCount().get(), 2);
    pool.Release(c1).get();
    EXPECT_EQ(pool.ReadyCount().get(), 3);
}

TEST(ClientPoolTest, ReadyCountAsync)
{
    x::pool::IoContextPool io_context_pool;
    io_context_pool.Start(1);
    x::pool::ClientPool<x::http::Client> pool(io_context_pool.io_context());
    pool.SetClientCreateFactory([](boost::asio::io_context& ioc) { return new x::http::Client(ioc); });

    auto* c1 = pool.GetClient().get();
    auto* c2 = pool.GetClient().get();
    auto* c3 = pool.GetClient().get();

    std::latch latch0(1);
    pool.ReadyCount([&latch0](const int ready_count)
    {
        EXPECT_EQ(ready_count, 0);
        latch0.count_down();
    });
    latch0.wait();

    std::latch latch1(1);
    pool.Release(c1).get();
    pool.ReadyCount([&latch1](const int ready_count)
    {
        EXPECT_EQ(ready_count, 1);
        latch1.count_down();
    });
    latch1.wait();

    std::latch latch2(1);
    pool.Release(c2).get();
    pool.ReadyCount([&latch2](const int ready_count)
    {
        EXPECT_EQ(ready_count, 2);
        latch2.count_down();
    });
    latch2.wait();

    std::latch latch3(1);
    pool.Release(c3).get();
    pool.ReadyCount([&latch3](const int ready_count)
    {
        EXPECT_EQ(ready_count, 3);
        latch3.count_down();
    });
    latch3.wait();
}

TEST(ClientPoolTest, InUseCountFuture)
{
    x::pool::IoContextPool io_context_pool;
    io_context_pool.Start(1);
    x::pool::ClientPool<x::http::Client> pool(io_context_pool.io_context());
    pool.SetClientCreateFactory([](boost::asio::io_context& ioc) { return new x::http::Client(ioc); });

    EXPECT_EQ(pool.InUseCount().get(), 0);
    auto* c1 = pool.GetClient().get();
    auto* c2 = pool.GetClient().get();
    auto* c3 = pool.GetClient().get();
    EXPECT_EQ(pool.InUseCount().get(), 3);

    pool.Release(c1).get();
    EXPECT_EQ(pool.InUseCount().get(), 2);
    pool.Release(c2).get();
    EXPECT_EQ(pool.InUseCount().get(), 1);
    pool.Release(c3).get();
    EXPECT_EQ(pool.InUseCount().get(), 0);

    c1 = pool.GetClient().get();
    EXPECT_EQ(pool.InUseCount().get(), 1);
    pool.Release(c1).get();
    EXPECT_EQ(pool.InUseCount().get(), 0);

    c1 = pool.GetClient().get();
    EXPECT_EQ(pool.InUseCount().get(), 1);
    c2 = pool.GetClient().get();
    EXPECT_EQ(pool.InUseCount().get(), 2);
    pool.Release(c2).get();
    EXPECT_EQ(pool.InUseCount().get(), 1);
    pool.Release(c1).get();
    EXPECT_EQ(pool.InUseCount().get(), 0);

    c1 = pool.GetClient().get();
    EXPECT_EQ(pool.InUseCount().get(), 1);
    c2 = pool.GetClient().get();
    EXPECT_EQ(pool.InUseCount().get(), 2);
    c3 = pool.GetClient().get();
    EXPECT_EQ(pool.InUseCount().get(), 3);
    pool.Release(c3).get();
    EXPECT_EQ(pool.InUseCount().get(), 2);
    pool.Release(c2).get();
    EXPECT_EQ(pool.InUseCount().get(), 1);
    pool.Release(c1).get();
    EXPECT_EQ(pool.InUseCount().get(), 0);
}

TEST(ClientPoolTest, InUseCountAsync)
{
    x::pool::IoContextPool io_context_pool;
    io_context_pool.Start(1);
    x::pool::ClientPool<x::http::Client> pool(io_context_pool.io_context());
    pool.SetClientCreateFactory([](boost::asio::io_context& ioc) { return new x::http::Client(ioc); });

    auto* c1 = pool.GetClient().get();
    auto* c2 = pool.GetClient().get();
    auto* c3 = pool.GetClient().get();

    std::latch latch0(1);
    pool.InUseCount([&latch0](const int in_use_count)
    {
        EXPECT_EQ(in_use_count, 3);
        latch0.count_down();
    });
    latch0.wait();

    std::latch latch1(1);
    pool.Release(c1).get();
    pool.InUseCount([&latch1](const int in_use_count)
    {
        EXPECT_EQ(in_use_count, 2);
        latch1.count_down();
    });
    latch1.wait();

    std::latch latch2(1);
    pool.Release(c2).get();
    pool.InUseCount([&latch2](const int in_use_count)
    {
        EXPECT_EQ(in_use_count, 1);
        latch2.count_down();
    });
    latch2.wait();

    std::latch latch3(1);
    pool.Release(c3).get();
    pool.InUseCount([&latch3](const int in_use_count)
    {
        EXPECT_EQ(in_use_count, 0);
        latch3.count_down();
    });
    latch3.wait();
}

TEST(ClientPoolTest, CancelAllFuture)
{
    x::pool::IoContextPool io_context_pool;
    io_context_pool.Start(1);
    x::pool::ClientPool<x::http::Client> pool(io_context_pool.io_context());
    pool.SetClientCreateFactory([](boost::asio::io_context& ioc)
    {
        auto* client = new x::http::Client(ioc);
        client->host = "www.baidu.com";
        client->port = "80";
        auto& req = client->request.New<x::http::Request::StringBody>();
        req.method(boost::beast::http::verb::get);
        req.target("/");
        req.version(11);
        client->response.New<x::http::Response::StringBody>();
        return client;
    });

    auto* c1 = pool.GetClient().get();
    auto* c2 = pool.GetClient().get();
    auto* c3 = pool.GetClient().get();

    c1->Start([c1]
    {
        EXPECT_EQ(c1->error_code().value(), boost::beast::errc::operation_canceled);
    });
    c2->Start([c2]
    {
        EXPECT_EQ(c2->error_code().value(), boost::beast::errc::operation_canceled);
    });
    c3->Start([c3]
    {
        EXPECT_EQ(c3->error_code().value(), boost::beast::errc::operation_canceled);
    });

    const int cancelled_count = pool.CancelAll().get();
    EXPECT_EQ(cancelled_count, 3);
}

TEST(ClientPoolTest, CancelAllAsync)
{
    x::pool::IoContextPool io_context_pool;
    io_context_pool.Start(1);
    x::pool::ClientPool<x::http::Client> pool(io_context_pool.io_context());
    pool.SetClientCreateFactory([](boost::asio::io_context& ioc)
    {
        auto* client = new x::http::Client(ioc);
        client->host = "www.baidu.com";
        client->port = "80";
        auto& req = client->request.New<x::http::Request::StringBody>();
        req.method(boost::beast::http::verb::get);
        req.target("/");
        req.version(11);
        client->response.New<x::http::Response::StringBody>();
        return client;
    });

    std::latch latch(4);
    auto* c1 = pool.GetClient().get();
    auto* c2 = pool.GetClient().get();
    auto* c3 = pool.GetClient().get();

    c1->Start([c1, &latch]
    {
        EXPECT_EQ(c1->error_code().value(), boost::beast::errc::operation_canceled);
        latch.count_down();
    });
    c2->Start([c2, &latch]
    {
        EXPECT_EQ(c2->error_code().value(), boost::beast::errc::operation_canceled);
        latch.count_down();
    });
    c3->Start([c3, &latch]
    {
        EXPECT_EQ(c3->error_code().value(), boost::beast::errc::operation_canceled);
        latch.count_down();
    });

    pool.CancelAll([&latch](const int cancelled_count)
    {
        EXPECT_EQ(cancelled_count, 3);
        latch.count_down();
    });

    latch.wait();
}

TEST(ClientPoolTest, WaitAllReady)
{
    x::pool::IoContextPool io_context_pool;
    io_context_pool.Start(1);
    x::pool::ClientPool<x::http::Client> pool(io_context_pool.io_context());
    pool.SetClientCreateFactory([](boost::asio::io_context& ioc)
    {
        auto* client = new x::http::Client(ioc);
        client->host = "www.baidu.com";
        client->port = "80";
        auto& req = client->request.New<x::http::Request::StringBody>();
        req.method(boost::beast::http::verb::get);
        req.target("/");
        req.version(11);
        client->response.New<x::http::Response::StringBody>();
        return client;
    });

    auto* c1 = pool.GetClient().get();
    auto* c2 = pool.GetClient().get();
    auto* c3 = pool.GetClient().get();

    c1->Start([c1, &pool]
    {
        EXPECT_EQ(c1->error_code().value(), boost::beast::errc::operation_canceled);
        pool.Release(c1, [](bool){});
    });
    c2->Start([c2, &pool]
    {
        EXPECT_EQ(c2->error_code().value(), boost::beast::errc::operation_canceled);
        pool.Release(c2, [](bool){});
    });
    c3->Start([c3, &pool]
    {
        EXPECT_EQ(c3->error_code().value(), boost::beast::errc::operation_canceled);
        pool.Release(c3, [](bool){});
    });

    pool.CancelAll([](const int cancelled_count)
    {
        EXPECT_EQ(cancelled_count, 3);
    });

    pool.WaitAllReady();
    EXPECT_EQ(pool.ReadyCount().get(), 3);
    EXPECT_EQ(pool.InUseCount().get(), 0);
}
