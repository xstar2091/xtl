#include <gtest/gtest.h>

#include "x/network/http/request.h"

TEST(RequestTest, Get)
{
    x::http::Request request;

    request.New<x::http::Request::BufferBody>();
    auto& req_buffer_body = request.Get<x::http::Request::BufferBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_buffer_body)>, x::http::Request::BufferBody>));
    request.New<x::http::Request::DynamicBody>();
    auto& req_dynamic_body = request.Get<x::http::Request::DynamicBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_dynamic_body)>, x::http::Request::DynamicBody>));
    request.New<x::http::Request::EmptyBody>();
    auto& req_empty_body = request.Get<x::http::Request::EmptyBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_empty_body)>, x::http::Request::EmptyBody>));
    request.New<x::http::Request::FileBody>();
    auto& req_file_body = request.Get<x::http::Request::FileBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_file_body)>, x::http::Request::FileBody>));
    request.New<x::http::Request::SpanBody>();
    auto& req_span_body = request.Get<x::http::Request::SpanBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_span_body)>, x::http::Request::SpanBody>));
    request.New<x::http::Request::StringBody>();
    auto& req_string_body = request.Get<x::http::Request::StringBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_string_body)>, x::http::Request::StringBody>));
}

TEST(RequestTest, New)
{
    x::http::Request request;

    auto& req_buffer_body = request.New<x::http::Request::BufferBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_buffer_body)>, x::http::Request::BufferBody>));
    auto& req_dynamic_body = request.New<x::http::Request::DynamicBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_dynamic_body)>, x::http::Request::DynamicBody>));
    auto& req_empty_body = request.New<x::http::Request::EmptyBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_empty_body)>, x::http::Request::EmptyBody>));
    auto& req_file_body = request.New<x::http::Request::FileBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_file_body)>, x::http::Request::FileBody>));
    auto& req_span_body = request.New<x::http::Request::SpanBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_span_body)>, x::http::Request::SpanBody>));
    auto& req_string_body = request.New<x::http::Request::StringBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_string_body)>, x::http::Request::StringBody>));
}

TEST(RequestTest, Reset)
{
    x::http::Request request;

    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(request.Get<std::monostate>())>, std::monostate>));
    auto& req_string_body = request.New<x::http::Request::StringBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(req_string_body)>, x::http::Request::StringBody>));
    request.Reset();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(request.Get<std::monostate>())>, std::monostate>));
}
