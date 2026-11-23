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
