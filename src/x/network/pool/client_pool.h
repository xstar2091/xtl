#pragma once

#include <future>
#include <unordered_set>
#include <vector>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/io_context.hpp>

namespace x::pool
{

template <class ClientType>
class ClientPool
{
    std::vector<ClientType*> ready_pool_;
    std::unordered_set<ClientType*> in_use_pool_;
    boost::asio::io_context& io_context_;
    std::function<ClientType*(boost::asio::io_context&)> client_create_factory_;

public:
    explicit ClientPool(boost::asio::io_context& io_context);
    ~ClientPool();

    void SetClientCreateFactory(std::function<ClientType*(boost::asio::io_context&)> client_create_factory);

    [[nodiscard]] boost::asio::io_context& io_context() { return io_context_; }
    [[nodiscard]] const boost::asio::io_context& io_context() const { return io_context_; }

    // NOLINTNEXTLINE(modernize-use-nodiscard)
    std::future<int> ReadyCount() const;
    template <class ReadyCountCallbackType>
    void ReadyCount(ReadyCountCallbackType&& callback) const;
    // NOLINTNEXTLINE(modernize-use-nodiscard)
    std::future<int> InUseCount() const;
    template <class InUseCountCallbackType>
    void InUseCount(InUseCountCallbackType&& callback) const;

    std::future<ClientType*> GetClient();
    template <class GetClientCallbackType>
    void GetClient(GetClientCallbackType&& callback);
    std::future<bool> Release(ClientType* client);
    template <class ReleaseClientCallbackType>
    void Release(ClientType* client, ReleaseClientCallbackType&& callback);
    std::future<int> CancelAll();
    template <class CancelAllCallbackType>
    void CancelAll(CancelAllCallbackType&& callback);
    void WaitAllReady() const;
};

template <class ClientType>
ClientPool<ClientType>::ClientPool(boost::asio::io_context& io_context)
    : io_context_(io_context)
{
}

template <class ClientType>
ClientPool<ClientType>::~ClientPool()
{
    for (auto& client : in_use_pool_)
    {
        delete client;
    }
    in_use_pool_.clear();
    for (auto& client : ready_pool_)
    {
        delete client;
    }
    ready_pool_.clear();
}

template <class ClientType>
void ClientPool<ClientType>::SetClientCreateFactory(
    std::function<ClientType*(boost::asio::io_context&)> client_create_factory
)
{
    client_create_factory_ = std::move(client_create_factory);
}

template <class ClientType>
std::future<int> ClientPool<ClientType>::ReadyCount() const
{
    std::promise<int> promise;
    auto future = promise.get_future();
    boost::asio::dispatch(
        io_context_,
        [this, promise = std::move(promise)]() mutable
        {
            promise.set_value(static_cast<int>(ready_pool_.size()));
        }
    );
    return future;
}

template <class ClientType>
template <class ReadyCountCallbackType>
void ClientPool<ClientType>::ReadyCount(ReadyCountCallbackType&& callback) const
{
    boost::asio::dispatch(
        io_context_,
        [this, callback = std::forward<ReadyCountCallbackType>(callback)]() mutable
        {
            callback(static_cast<int>(ready_pool_.size()));
        }
    );
}

template <class ClientType>
std::future<int> ClientPool<ClientType>::InUseCount() const
{
    std::promise<int> promise;
    auto future = promise.get_future();
    boost::asio::dispatch(
        io_context_,
        [this, promise = std::move(promise)]() mutable
        {
            promise.set_value(static_cast<int>(in_use_pool_.size()));
        }
    );
    return future;
}

template <class ClientType>
template <class InUseCountCallbackType>
void ClientPool<ClientType>::InUseCount(InUseCountCallbackType&& callback) const
{
    boost::asio::dispatch(
        io_context_,
        [this, callback = std::forward<InUseCountCallbackType>(callback)]() mutable
        {
            callback(static_cast<int>(in_use_pool_.size()));
        }
    );
}

template <class ClientType>
std::future<ClientType*> ClientPool<ClientType>::GetClient()
{
    std::promise<ClientType*> promise;
    auto future = promise.get_future();
    boost::asio::dispatch(
        io_context_,
        [this, promise = std::move(promise)]() mutable
        {
            ClientType* client = nullptr;
            if (ready_pool_.empty())
            {
                client = client_create_factory_(io_context_);
            }
            else
            {
                client = ready_pool_.back();
                ready_pool_.pop_back();
            }
            in_use_pool_.insert(client);
            promise.set_value(client);
        }
    );
    return future;
}

template <class ClientType>
template <class GetClientCallbackType>
void ClientPool<ClientType>::GetClient(GetClientCallbackType&& callback)
{
    boost::asio::dispatch(
        io_context_,
        [this, callback = std::forward<GetClientCallbackType>(callback)]
        {
            ClientType* client = nullptr;
            if (ready_pool_.empty())
            {
                client = client_create_factory_(io_context_);
            }
            else
            {
                client = ready_pool_.back();
                ready_pool_.pop_back();
            }
            in_use_pool_.insert(client);
            callback(client);
        }
    );
}

template <class ClientType>
std::future<bool> ClientPool<ClientType>::Release(ClientType* client)
{
    std::promise<bool> promise;
    auto future = promise.get_future();
    boost::asio::dispatch(
        io_context_,
        [this, client, promise = std::move(promise)]() mutable
        {
            bool result = false;
            if (const auto it = in_use_pool_.find(client); it == in_use_pool_.end())
            {
                delete client;
            }
            else
            {
                result = true;
                in_use_pool_.erase(it);
                ready_pool_.push_back(client);
            }
            promise.set_value(result);
        }
    );
    return future;
}

template <class ClientType>
template <class ReleaseClientCallbackType>
void ClientPool<ClientType>::Release(ClientType* client, ReleaseClientCallbackType&& callback)
{
    boost::asio::dispatch(
        io_context_,
        [this, client, callback = std::forward<ReleaseClientCallbackType>(callback)]() mutable
        {
            bool result = false;
            if (const auto it = in_use_pool_.find(client); it == in_use_pool_.end())
            {
                delete client;
            }
            else
            {
                result = true;
                in_use_pool_.erase(it);
                ready_pool_.push_back(client);
            }
            callback(result);
        }
    );
}

template <class ClientType>
std::future<int> ClientPool<ClientType>::CancelAll()
{
    std::promise<int> promise;
    auto future = promise.get_future();
    boost::asio::dispatch(
        io_context_,
        [this, promise = std::move(promise)]() mutable
        {
            const int result = static_cast<int>(in_use_pool_.size());
            for (auto& client : in_use_pool_)
            {
                client.Cancel();
            }
            promise.set_value(result);
        }
    );
    return future;
}

template <class ClientType>
template <class CancelAllCallbackType>
void ClientPool<ClientType>::CancelAll(CancelAllCallbackType&& callback)
{
    boost::asio::dispatch(
        io_context_,
        [this, callback = std::forward<CancelAllCallbackType>(callback)]
        {
            const int result = static_cast<int>(in_use_pool_.size());
            for (auto& client : in_use_pool_)
            {
                client.Cancel();
            }
            callback(result);
        }
    );
}

template <class ClientType>
void ClientPool<ClientType>::WaitAllReady() const
{
    int in_use_count = InUseCount().get();
    while (in_use_count)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        in_use_count = InUseCount().get();
    }
}

}
