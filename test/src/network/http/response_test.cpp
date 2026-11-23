#include <gtest/gtest.h>

#include "x/network/http/response.h"

TEST(ResponseTest, Get)
{
    x::http::Response response;

    response.New<x::http::Response::BufferBody>();
    auto& resp_buffer_body = response.Get<x::http::Response::BufferBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_buffer_body)>, x::http::Response::BufferBody>));
    response.New<x::http::Response::DynamicBody>();
    auto& resp_dynamic_body = response.Get<x::http::Response::DynamicBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_dynamic_body)>, x::http::Response::DynamicBody>));
    response.New<x::http::Response::EmptyBody>();
    auto& resp_empty_body = response.Get<x::http::Response::EmptyBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_empty_body)>, x::http::Response::EmptyBody>));
    response.New<x::http::Response::FileBody>();
    auto& resp_file_body = response.Get<x::http::Response::FileBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_file_body)>, x::http::Response::FileBody>));
    response.New<x::http::Response::SpanBody>();
    auto& resp_span_body = response.Get<x::http::Response::SpanBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_span_body)>, x::http::Response::SpanBody>));
    response.New<x::http::Response::StringBody>();
    auto& resp_string_body = response.Get<x::http::Response::StringBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_string_body)>, x::http::Response::StringBody>));
}

TEST(ResponseTest, New)
{
    x::http::Response response;

    auto& resp_buffer_body = response.New<x::http::Response::BufferBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_buffer_body)>, x::http::Response::BufferBody>));
    auto& resp_dynamic_body = response.New<x::http::Response::DynamicBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_dynamic_body)>, x::http::Response::DynamicBody>));
    auto& resp_empty_body = response.New<x::http::Response::EmptyBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_empty_body)>, x::http::Response::EmptyBody>));
    auto& resp_file_body = response.New<x::http::Response::FileBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_file_body)>, x::http::Response::FileBody>));
    auto& resp_span_body = response.New<x::http::Response::SpanBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_span_body)>, x::http::Response::SpanBody>));
    auto& resp_string_body = response.New<x::http::Response::StringBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_string_body)>, x::http::Response::StringBody>));
}

TEST(ResponseTest, Reset)
{
    x::http::Response response;

    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(response.Get<std::monostate>())>, std::monostate>));
    auto& resp_string_body = response.New<x::http::Response::StringBody>();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(resp_string_body)>, x::http::Response::StringBody>));
    response.Reset();
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(response.Get<std::monostate>())>, std::monostate>));
}

TEST(ResponseTest, StatusCode)
{
    x::http::Response response;

    EXPECT_EQ(response.StatusCode(), 0);
    auto& resp = response.New<x::http::Response::StringBody>();
    resp.result(boost::beast::http::status::ok);
    EXPECT_EQ(response.StatusCode(), static_cast<int>(boost::beast::http::status::ok));
    resp.result(boost::beast::http::status::not_found);
    EXPECT_EQ(response.StatusCode(), static_cast<int>(boost::beast::http::status::not_found));
    resp.result(boost::beast::http::status::internal_server_error);
    EXPECT_EQ(response.StatusCode(), static_cast<int>(boost::beast::http::status::internal_server_error));
    response.Reset();
    EXPECT_EQ(response.StatusCode(), 0);
}
