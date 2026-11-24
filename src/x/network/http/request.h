#pragma once

#include <variant>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/span_body.hpp>
#include <boost/beast/http/string_body.hpp>

namespace x::http
{

class Request
{
public:
    struct BufferBody : boost::beast::http::request<boost::beast::http::buffer_body> {};
    struct DynamicBody : boost::beast::http::request<boost::beast::http::dynamic_body> {};
    struct EmptyBody : boost::beast::http::request<boost::beast::http::empty_body> {};
    struct FileBody : boost::beast::http::request<boost::beast::http::file_body> {};
    struct SpanBody : boost::beast::http::request<boost::beast::http::span_body<char>> {};
    struct StringBody : boost::beast::http::request<boost::beast::http::string_body> {};

    template<class BodyType>
    [[nodiscard]] BodyType& Get()
    {
        return std::get<BodyType>(request_);
    }

    template<class BodyType>
    BodyType& New()
    {
        request_ = BodyType();
        return std::get<BodyType>(request_);
    }

    [[nodiscard]] std::variant<std::monostate, BufferBody, DynamicBody, EmptyBody, FileBody, SpanBody, StringBody>& Raw()
    {
        return request_;
    }

    void Reset() { request_ = std::monostate(); }

private:
    std::variant<std::monostate, BufferBody, DynamicBody, EmptyBody, FileBody, SpanBody, StringBody> request_;
};

}
