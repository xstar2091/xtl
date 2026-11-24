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

class Response
{
public:
    struct BufferBody : boost::beast::http::response<boost::beast::http::buffer_body> {};
    struct DynamicBody : boost::beast::http::response<boost::beast::http::dynamic_body> {};
    struct EmptyBody : boost::beast::http::response<boost::beast::http::empty_body> {};
    struct FileBody : boost::beast::http::response<boost::beast::http::file_body> {};
    struct SpanBody : boost::beast::http::response<boost::beast::http::span_body<char>> {};
    struct StringBody : boost::beast::http::response<boost::beast::http::string_body> {};

    template<class BodyType>
    [[nodiscard]] BodyType& Get()
    {
        return std::get<BodyType>(response_);
    }

    template<class BodyType>
    BodyType& New()
    {
        response_ = BodyType();
        return std::get<BodyType>(response_);
    }

    [[nodiscard]] std::variant<std::monostate, BufferBody, DynamicBody, EmptyBody, FileBody, SpanBody, StringBody>& Raw()
    {
        return response_;
    }

    void Reset() { response_ = std::monostate(); }

    [[nodiscard]] int StatusCode() const;

private:
    std::variant<std::monostate, BufferBody, DynamicBody, EmptyBody, FileBody, SpanBody, StringBody> response_;
};

}
