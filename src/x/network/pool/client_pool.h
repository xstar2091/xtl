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

    [[nodiscard]] std::future<int> ReadyCount() const;
    void ReadyCount(std::move_only_function<void(int)> callback) const;
    [[nodiscard]] std::future<int> InUseCount() const;
    void InUseCount(std::move_only_function<void(int)> callback) const;

    [[nodiscard]] std::future<ClientType*> GetClient();
    void GetClient(std::move_only_function<void(ClientType*)> callback);
    std::future<bool> Release(ClientType* client);
    void Release(ClientType* client, std::move_only_function<void(bool)> callback);
    std::future<int> CancelAll();
    void CancelAll(std::move_only_function<void(int)> callback);
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
    ReadyCount([promise = std::move(promise)](const int ready_count) mutable { promise.set_value(ready_count); });
    return future;
}

template <class ClientType>
void ClientPool<ClientType>::ReadyCount(std::move_only_function<void(int)> callback) const
{
    boost::asio::dispatch(
        io_context_,
        [this, callback = std::move(callback)]() mutable
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
    InUseCount([promise = std::move(promise)](const int in_use_count) mutable { promise.set_value(in_use_count); });
    return future;
}

template <class ClientType>
void ClientPool<ClientType>::InUseCount(std::move_only_function<void(int)> callback) const
{
    boost::asio::dispatch(
        io_context_,
        [this, callback = std::move(callback)]() mutable
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
    GetClient([promise = std::move(promise)](ClientType* client) mutable{ promise.set_value(client); });
    return future;
}

template <class ClientType>
void ClientPool<ClientType>::GetClient(std::move_only_function<void(ClientType*)> callback)
{
    boost::asio::dispatch(
        io_context_,
        [this, callback = std::move(callback)] mutable
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
    Release(client, [promise = std::move(promise)](const bool result) mutable { promise.set_value(result); });
    return future;
}

template <class ClientType>
void ClientPool<ClientType>::Release(ClientType* client, std::move_only_function<void(bool)> callback)
{
    boost::asio::dispatch(
        io_context_,
        [this, client, callback = std::move(callback)]() mutable
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
    CancelAll([promise = std::move(promise)](const int cancelled_count) mutable { promise.set_value(cancelled_count); });
    return future;
}

template <class ClientType>
void ClientPool<ClientType>::CancelAll(std::move_only_function<void(int)> callback)
{
    boost::asio::dispatch(
        io_context_,
        [this, callback = std::move(callback)]() mutable
        {
            const int result = static_cast<int>(in_use_pool_.size());
            for (auto* client : in_use_pool_)
            {
                client->Cancel();
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
