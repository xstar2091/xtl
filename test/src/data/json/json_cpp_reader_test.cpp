#include <gtest/gtest.h>

#include "x/data/json/json_cpp_reader.h"
#include "utils/data/json/json_cpp_data.h"

TEST(JsonCppReaderTest, LineSuccess)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    auto [type, day_use, month_use, user_number, average_price] = json.Line<
        std::string, bool, bool, int, double
    >("type", "day_use", "month_use", "user_number", "average_price");
    EXPECT_FALSE(json.Error());
    EXPECT_TRUE(json.ErrorMessage().empty());
    EXPECT_EQ(type, data.json["type"].asString());
    EXPECT_EQ(day_use, data.json["day_use"].asBool());
    EXPECT_EQ(month_use, data.json["month_use"].asBool());
    EXPECT_EQ(user_number, data.json["user_number"].asInt());
    EXPECT_DOUBLE_EQ(average_price, data.json["average_price"].asDouble());
}

TEST(JsonCppReaderTest, LineError)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    [[maybe_unused]] auto [type, day_use, month_use, user_number, average_price] = json.Line<
        std::string, bool, bool, int, double
    >("type", "day_use_NOT_EXIST", "month_use", "user_number", "average_price");
    EXPECT_TRUE(json.Error());
    EXPECT_FALSE(json.ErrorMessage().empty());
}

TEST(JsonCppReaderTest, ClearError)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    [[maybe_unused]] auto [type, day_use, month_use, user_number, average_price] = json.Line<
        std::string, bool, bool, int, double
    >("type", "day_use_NOT_EXIST", "month_use", "user_number", "average_price");
    EXPECT_TRUE(json.Error());
    EXPECT_FALSE(json.ErrorMessage().empty());
    json.ClearError();
    EXPECT_FALSE(json.Error());
    EXPECT_TRUE(json.ErrorMessage().empty());
}

TEST(JsonCppReaderTest, CreateReaderWithDefaultPolicy)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    const x::data::JsonCppReader::Reader reader = json.CreateReader().SetDefaultPolicy();
    auto [type, day_use, month_use, user_number, average_price] = reader.Line<
        std::string, bool, bool, int, double
    >("type", "day_use", "month_use", "user_number", "average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());

    EXPECT_EQ(type, data.json["type"].asString());
    EXPECT_EQ(day_use, data.json["day_use"].asBool());
    EXPECT_EQ(month_use, data.json["month_use"].asBool());
    EXPECT_EQ(user_number, data.json["user_number"].asInt());
    EXPECT_DOUBLE_EQ(average_price, data.json["average_price"].asDouble());
}

TEST(JsonCppReaderTest, CreateReaderWithDefaultPolicyWithNotExist_1)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    const x::data::JsonCppReader::Reader reader = json.CreateReader().SetDefaultPolicy();
    auto [type, day_use, month_use, user_number, average_price, val_str, val_int, val_bool, val_double] = reader.Line<
        std::string, bool, bool, int, double, std::string, int, bool, double
    >(
        "type", "day_use", "month_use", "user_number", "average_price",
        "string_value_not_exist", "int_value_not_exist", "bool_value_not_exist", "double_value_not_exist"
    );
    EXPECT_TRUE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());

    EXPECT_EQ(type, data.json["type"].asString());
    EXPECT_EQ(day_use, data.json["day_use"].asBool());
    EXPECT_EQ(month_use, data.json["month_use"].asBool());
    EXPECT_EQ(user_number, data.json["user_number"].asInt());
    EXPECT_DOUBLE_EQ(average_price, data.json["average_price"].asDouble());

    EXPECT_EQ(val_str, std::string());
    EXPECT_EQ(val_int, int());
    EXPECT_EQ(val_bool, bool());
    EXPECT_DOUBLE_EQ(val_double, double());
}

TEST(JsonCppReaderTest, CreateReaderWithDefaultPolicyWithNotExist_2)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    const x::data::JsonCppReader::Reader reader = json.CreateReader().SetDefaultPolicy();
    auto [type, val_str, day_use, val_bool, month_use, val_int, user_number, average_price, val_double] = reader.Line<
        std::string, std::string, bool, bool, bool, int, int, double, double
    >(
        "type", "string_value_not_exist", "day_use", "bool_value_not_exist", "month_use",
        "int_value_not_exist", "user_number", "average_price", "double_value_not_exist"
    );
    EXPECT_TRUE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());

    EXPECT_EQ(type, data.json["type"].asString());
    EXPECT_EQ(day_use, data.json["day_use"].asBool());
    EXPECT_EQ(month_use, data.json["month_use"].asBool());
    EXPECT_EQ(user_number, data.json["user_number"].asInt());
    EXPECT_DOUBLE_EQ(average_price, data.json["average_price"].asDouble());

    EXPECT_EQ(val_str, std::string());
    EXPECT_EQ(val_int, int());
    EXPECT_EQ(val_bool, bool());
    EXPECT_DOUBLE_EQ(val_double, double());
}

TEST(JsonCppReaderTest, CreateReaderWithErrorPolicy)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    const x::data::JsonCppReader::Reader reader = json.CreateReader().SetErrorPolicy();
    auto [type, day_use, month_use, user_number, average_price] = reader.Line<
        std::string, bool, bool, int, double
    >("type", "day_use", "month_use", "user_number", "average_price");
    EXPECT_FALSE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());

    EXPECT_EQ(type, data.json["type"].asString());
    EXPECT_EQ(day_use, data.json["day_use"].asBool());
    EXPECT_EQ(month_use, data.json["month_use"].asBool());
    EXPECT_EQ(user_number, data.json["user_number"].asInt());
    EXPECT_DOUBLE_EQ(average_price, data.json["average_price"].asDouble());
}

TEST(JsonCppReaderTest, CreateReaderWithErrorPolicyWithNotExist_1)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    const x::data::JsonCppReader::Reader reader = json.CreateReader().SetErrorPolicy();
    auto [type, day_use, month_use, user_number, average_price, val_str, val_int, val_bool, val_double] = reader.Line<
        std::string, bool, bool, int, double, std::string, int, bool, double
    >(
        "type", "day_use", "month_use", "user_number", "average_price",
        "string_value_not_exist", "int_value_not_exist", "bool_value_not_exist", "double_value_not_exist"
    );
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());

    EXPECT_EQ(type, data.json["type"].asString());
    EXPECT_EQ(day_use, data.json["day_use"].asBool());
    EXPECT_EQ(month_use, data.json["month_use"].asBool());
    EXPECT_EQ(user_number, data.json["user_number"].asInt());
    EXPECT_DOUBLE_EQ(average_price, data.json["average_price"].asDouble());

    EXPECT_EQ(val_str, std::string());
    EXPECT_EQ(val_int, int());
    EXPECT_EQ(val_bool, bool());
    EXPECT_DOUBLE_EQ(val_double, double());
}

TEST(JsonCppReaderTest, CreateReaderWithErrorPolicyWithNotExist_2)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    const x::data::JsonCppReader::Reader reader = json.CreateReader().SetErrorPolicy();
    auto [type, val_str, day_use, val_bool, month_use, val_int, user_number, average_price, val_double] = reader.Line<
        std::string, std::string, bool, bool, bool, int, int, double, double
    >(
        "type", "string_value_not_exist", "day_use", "bool_value_not_exist", "month_use",
        "int_value_not_exist", "user_number", "average_price", "double_value_not_exist"
    );
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());

    EXPECT_EQ(type, data.json["type"].asString());
    EXPECT_EQ(day_use, data.json["day_use"].asBool());
    EXPECT_EQ(month_use, data.json["month_use"].asBool());
    EXPECT_EQ(user_number, data.json["user_number"].asInt());
    EXPECT_DOUBLE_EQ(average_price, data.json["average_price"].asDouble());

    EXPECT_EQ(val_str, std::string());
    EXPECT_EQ(val_int, int());
    EXPECT_EQ(val_bool, bool());
    EXPECT_DOUBLE_EQ(val_double, double());
}

TEST(JsonCppReaderTest, CreateReaderWithExceptionPolicy)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    const x::data::JsonCppReader::Reader reader = json.CreateReader().SetExceptionPolicy();
    auto [type, day_use, month_use, user_number, average_price] = reader.Line<
        std::string, bool, bool, int, double
    >("type", "day_use", "month_use", "user_number", "average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());

    EXPECT_EQ(type, data.json["type"].asString());
    EXPECT_EQ(day_use, data.json["day_use"].asBool());
    EXPECT_EQ(month_use, data.json["month_use"].asBool());
    EXPECT_EQ(user_number, data.json["user_number"].asInt());
    EXPECT_DOUBLE_EQ(average_price, data.json["average_price"].asDouble());
}

TEST(JsonCppReaderTest, CreateReaderWithExceptionPolicyWithNotExist_1)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    const x::data::JsonCppReader::Reader reader = json.CreateReader().SetExceptionPolicy();
    EXPECT_THROW((reader.Line<
        std::string, bool, bool, int, double, std::string, int, bool, double
    >(
        "type", "day_use", "month_use", "user_number", "average_price",
        "string_value_not_exist", "int_value_not_exist", "bool_value_not_exist", "double_value_not_exist"
    )), std::runtime_error);
    EXPECT_TRUE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());
}

TEST(JsonCppReaderTest, CreateReaderWithExceptionPolicyWithNotExist_2)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    const x::data::JsonCppReader::Reader reader = json.CreateReader().SetExceptionPolicy();
    EXPECT_THROW((reader.Line<
        std::string, std::string, bool, bool, bool, int, int, double, double
    >(
        "type", "string_value_not_exist", "day_use", "bool_value_not_exist", "month_use",
        "int_value_not_exist", "user_number", "average_price", "double_value_not_exist"
    )), std::runtime_error);
    EXPECT_TRUE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());
}

TEST(JsonCppReaderTest, CreateReaderWithTryLine)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    const x::data::JsonCppReader::Reader reader = json.CreateReader().SetErrorPolicy();
    auto [type, day_use, month_use, user_number, average_price, val_str, val_int, val_bool, val_double] = reader.TryLine<
        std::string, bool, bool, int, double, std::string, int, bool, double
    >(
        "type", "day_use", "month_use", "user_number", "average_price",
        "string_value_not_exist", "int_value_not_exist", "bool_value_not_exist", "double_value_not_exist"
    );
    EXPECT_FALSE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());

    EXPECT_EQ(type.value(), data.json["type"].asString());
    EXPECT_EQ(day_use.value(), data.json["day_use"].asBool());
    EXPECT_EQ(month_use.value(), data.json["month_use"].asBool());
    EXPECT_EQ(user_number.value(), data.json["user_number"].asInt());
    EXPECT_DOUBLE_EQ(average_price.value(), data.json["average_price"].asDouble());

    EXPECT_FALSE(val_str.has_value());
    EXPECT_FALSE(val_int.has_value());
    EXPECT_FALSE(val_bool.has_value());
    EXPECT_FALSE(val_double.has_value());
}

TEST(JsonCppReaderTest, Scalar)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    auto reader = json.CreateReader().At("type");
    EXPECT_EQ(reader.Scalar<std::string>(), data.json["type"].asString());
    reader = json.CreateReader().At("day_use");
    EXPECT_EQ(reader.Scalar<bool>(), data.json["day_use"].asBool());
    reader = json.CreateReader().At("month_use");
    EXPECT_EQ(reader.Scalar<bool>(), data.json["month_use"].asBool());
    reader = json.CreateReader().At("user_number");
    EXPECT_EQ(reader.Scalar<int>(), data.json["user_number"].asInt());
    reader = json.CreateReader().At("average_price");
    EXPECT_EQ(reader.Scalar<double>(), data.json["average_price"].asDouble());
}

TEST(JsonCppReaderTest, ScalarKeyNotExist)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    auto reader = json.CreateReader().SetDefaultPolicy().At("key_not_exist");
    EXPECT_TRUE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());
    EXPECT_EQ(reader.Scalar<std::string>(), std::string());
    EXPECT_EQ(reader.Scalar<bool>(), bool());
    EXPECT_EQ(reader.Scalar<int>(), int());
    EXPECT_DOUBLE_EQ(reader.Scalar<double>(), double());
    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy().At("key_not_exist");
    EXPECT_EQ(reader.Scalar<std::string>(), std::string());
    EXPECT_EQ(reader.Scalar<bool>(), bool());
    EXPECT_EQ(reader.Scalar<int>(), int());
    EXPECT_DOUBLE_EQ(reader.Scalar<double>(), double());
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    json.ClearError();
    reader = json.CreateReader().SetExceptionPolicy().At("key_not_exist");
    EXPECT_TRUE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());
    EXPECT_THROW(reader.Scalar<std::string>(), std::runtime_error);
    EXPECT_THROW(reader.Scalar<bool>(), std::runtime_error);
    EXPECT_THROW(reader.Scalar<int>(), std::runtime_error);
    EXPECT_THROW(reader.Scalar<double>(), std::runtime_error);
}

TEST(JsonCppReaderTest, TryScalar)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    auto reader = json.CreateReader().At("type");
    EXPECT_EQ(reader.TryScalar<std::string>().value(), data.json["type"].asString());
    reader = json.CreateReader().At("day_use");
    EXPECT_EQ(reader.TryScalar<bool>().value(), data.json["day_use"].asBool());
    reader = json.CreateReader().At("month_use");
    EXPECT_EQ(reader.TryScalar<bool>().value(), data.json["month_use"].asBool());
    reader = json.CreateReader().At("user_number");
    EXPECT_EQ(reader.TryScalar<int>().value(), data.json["user_number"].asInt());
    reader = json.CreateReader().At("average_price");
    EXPECT_EQ(reader.TryScalar<double>().value(), data.json["average_price"].asDouble());
}

TEST(JsonCppReaderTest, TryScalarKeyNotExist)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    const auto reader = json.CreateReader().SetDefaultPolicy().At("key_not_exist");
    EXPECT_TRUE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());
    EXPECT_FALSE(reader.TryScalar<std::string>().has_value());
    EXPECT_FALSE(reader.TryScalar<bool>().has_value());
    EXPECT_FALSE(reader.TryScalar<int>().has_value());
    EXPECT_FALSE(reader.TryScalar<double>().has_value());
}

TEST(JsonCppReaderTest, IntNumber)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    auto reader = json.CreateReader();

    auto [val_bool] = reader.Line<bool>("user_number");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'user_number', expected bool, got int")
    );
    EXPECT_EQ(val_bool, false);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_short] = reader.Line<short>("user_number");
    EXPECT_EQ(val_short, static_cast<short>(data.json["user_number"].asInt()));
    auto [val_ushort] = reader.Line<unsigned short>("user_number");
    EXPECT_EQ(val_ushort, static_cast<unsigned short>(data.json["user_number"].asInt()));
    auto [val_int] = reader.Line<int>("user_number");
    EXPECT_EQ(val_int, data.json["user_number"].asInt());
    auto [val_uint] = reader.Line<unsigned int>("user_number");
    EXPECT_EQ(val_uint, data.json["user_number"].asUInt());
    auto [val_long] = reader.Line<long int>("user_number");
    EXPECT_EQ(val_long, data.json["user_number"].asInt64());
    auto [val_ulong] = reader.Line<unsigned long int>("user_number");
    EXPECT_EQ(val_ulong, data.json["user_number"].asUInt64());
    auto [val_longlong] = reader.Line<long long int>("user_number");
    EXPECT_EQ(val_longlong, data.json["user_number"].asInt64());
    auto [val_ulonglong] = reader.Line<unsigned long long int>("user_number");
    EXPECT_EQ(val_ulonglong, data.json["user_number"].asUInt64());

    auto [val_float] = reader.Line<float>("user_number");
    EXPECT_FLOAT_EQ(val_float, data.json["user_number"].asFloat());
    auto [val_double] = reader.Line<double>("user_number");
    EXPECT_DOUBLE_EQ(val_double, data.json["user_number"].asDouble());

    auto [val_int8] = reader.Line<int8_t>("user_number");
    EXPECT_EQ(val_int8, static_cast<int8_t>(data.json["user_number"].asInt()));
    auto [val_uint8] = reader.Line<uint8_t>("user_number");
    EXPECT_EQ(val_uint8, static_cast<uint8_t>(data.json["user_number"].asInt()));
    auto [val_int16] = reader.Line<int16_t>("user_number");
    EXPECT_EQ(val_int16, static_cast<int16_t>(data.json["user_number"].asInt()));
    auto [val_uint16] = reader.Line<uint16_t>("user_number");
    EXPECT_EQ(val_uint16, static_cast<uint16_t>(data.json["user_number"].asInt()));
    auto [val_int32] = reader.Line<int32_t>("user_number");
    EXPECT_EQ(val_int32, data.json["user_number"].asInt());
    auto [val_uint32] = reader.Line<uint32_t>("user_number");
    EXPECT_EQ(val_uint32, data.json["user_number"].asUInt());
    auto [val_int64] = reader.Line<int64_t>("user_number");
    EXPECT_EQ(val_int64, data.json["user_number"].asInt64());
    auto [val_uint64] = reader.Line<uint64_t>("user_number");
    EXPECT_EQ(val_uint64, data.json["user_number"].asUInt64());
}

TEST(JsonCppReaderTest, DoubleNumber)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    auto reader = json.CreateReader().SetErrorPolicy();

    auto [val_bool] = reader.Line<bool>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected bool, got real")
    );
    EXPECT_EQ(val_bool, false);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_short] = reader.Line<short>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected short, got real")
    );
    EXPECT_EQ(val_short, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_ushort] = reader.Line<unsigned short>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected ushort, got real")
    );
    EXPECT_EQ(val_ushort, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_int] = reader.Line<int>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected int, got real")
    );
    EXPECT_EQ(val_int, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_uint] = reader.Line<unsigned int>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected uint, got real")
    );
    EXPECT_EQ(val_uint, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_long] = reader.Line<long int>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected long, got real")
    );
    EXPECT_EQ(val_long, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_ulong] = reader.Line<unsigned long int>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected ulong, got real")
    );
    EXPECT_EQ(val_ulong, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_longlong] = reader.Line<long long int>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected longlong, got real")
    );
    EXPECT_EQ(val_longlong, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_ulonglong] = reader.Line<unsigned long long int>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected ulonglong, got real")
    );
    EXPECT_EQ(val_ulonglong, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());

    auto [val_float] = reader.Line<float>("average_price");
    EXPECT_FLOAT_EQ(val_float, data.json["average_price"].asFloat());
    auto [val_double] = reader.Line<double>("average_price");
    EXPECT_DOUBLE_EQ(val_double, data.json["average_price"].asDouble());

    auto [val_int8] = reader.Line<int8_t>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected int8_t, got real")
    );
    EXPECT_EQ(val_int8, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_uint8] = reader.Line<uint8_t>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected uint8_t, got real")
    );
    EXPECT_EQ(val_uint8, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_int16] = reader.Line<int16_t>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected short, got real")
    );
    EXPECT_EQ(val_int16, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_uint16] = reader.Line<uint16_t>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected ushort, got real")
    );
    EXPECT_EQ(val_uint16, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_int32] = reader.Line<int32_t>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected int, got real")
    );
    EXPECT_EQ(val_int32, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_uint32] = reader.Line<uint32_t>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected uint, got real")
    );
    EXPECT_EQ(val_uint32, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_int64] = reader.Line<int64_t>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected long, got real")
    );
    EXPECT_EQ(val_int64, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_uint64] = reader.Line<uint64_t>("average_price");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'average_price', expected ulong, got real")
    );
    EXPECT_EQ(val_uint64, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
}

TEST(JsonCppReaderTest, BoolNumber)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    auto reader = json.CreateReader().SetErrorPolicy();

    auto [val_bool] = reader.Line<bool>("month_use");
    EXPECT_FALSE(reader.Error());
    EXPECT_TRUE(reader.ErrorMessage().empty());
    EXPECT_EQ(val_bool, data.json["month_use"].asBool());
    auto [val_short] = reader.Line<short>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected short, got bool")
    );
    EXPECT_EQ(val_short, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_ushort] = reader.Line<unsigned short>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected ushort, got bool")
    );
    EXPECT_EQ(val_ushort, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_int] = reader.Line<int>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected int, got bool")
    );
    EXPECT_EQ(val_int, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_uint] = reader.Line<unsigned int>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected uint, got bool")
    );
    EXPECT_EQ(val_uint, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_long] = reader.Line<long int>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected long, got bool")
    );
    EXPECT_EQ(val_long, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_ulong] = reader.Line<unsigned long int>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected ulong, got bool")
    );
    EXPECT_EQ(val_ulong, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_longlong] = reader.Line<long long int>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected longlong, got bool")
    );
    EXPECT_EQ(val_longlong, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_ulonglong] = reader.Line<unsigned long long int>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected ulonglong, got bool")
    );
    EXPECT_EQ(val_ulonglong, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());

    auto [val_float] = reader.Line<float>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected float, got bool")
    );
    EXPECT_EQ(val_float, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_double] = reader.Line<double>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected double, got bool")
    );
    EXPECT_EQ(val_double, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());

    auto [val_int8] = reader.Line<int8_t>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected int8_t, got bool")
    );
    EXPECT_EQ(val_int8, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_uint8] = reader.Line<uint8_t>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected uint8_t, got bool")
    );
    EXPECT_EQ(val_uint8, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_int16] = reader.Line<int16_t>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected short, got bool")
    );
    EXPECT_EQ(val_int16, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_uint16] = reader.Line<uint16_t>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected ushort, got bool")
    );
    EXPECT_EQ(val_uint16, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_int32] = reader.Line<int32_t>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected int, got bool")
    );
    EXPECT_EQ(val_int32, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_uint32] = reader.Line<uint32_t>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected uint, got bool")
    );
    EXPECT_EQ(val_uint32, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_int64] = reader.Line<int64_t>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected long, got bool")
    );
    EXPECT_EQ(val_int64, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
    auto [val_uint64] = reader.Line<uint64_t>("month_use");
    EXPECT_TRUE(reader.Error());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'month_use', expected ulong, got bool")
    );
    EXPECT_EQ(val_uint64, 0);
    reader.ClearError();
    EXPECT_FALSE(reader.Error());
}

TEST(JsonCppReaderTest, String)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    const auto reader = json.CreateReader();

    auto [val_const_char_ptr] = reader.Line<const char*>("type");
    EXPECT_EQ(strcmp(val_const_char_ptr, data.json["type"].asCString()), 0);
    auto [val_string_view] = reader.Line<std::string_view>("type");
    EXPECT_EQ(val_string_view, std::string_view(data.json["type"].asCString()));
    EXPECT_EQ(val_string_view, std::string_view(data.json["type"].asString()));
    auto [val_string] = reader.Line<std::string>("type");
    EXPECT_EQ(val_string, data.json["type"].asString());

    const char* key_const_char_ptr = "type";
    constexpr std::string_view key_string_view("type");
    const std::string key_string("type");

    std::tie(val_const_char_ptr) = reader.Line<const char*>(key_const_char_ptr);
    EXPECT_EQ(strcmp(val_const_char_ptr, data.json["type"].asCString()), 0);
    std::tie(val_const_char_ptr) = reader.Line<const char*>(key_string_view);
    EXPECT_EQ(strcmp(val_const_char_ptr, data.json["type"].asCString()), 0);
    std::tie(val_const_char_ptr) = reader.Line<const char*>(key_string);
    EXPECT_EQ(strcmp(val_const_char_ptr, data.json["type"].asCString()), 0);

    std::tie(val_string_view) = reader.Line<std::string_view>(key_const_char_ptr);
    EXPECT_EQ(val_string_view, std::string_view(data.json["type"].asCString()));
    EXPECT_EQ(val_string_view, std::string_view(data.json["type"].asString()));
    std::tie(val_string_view) = reader.Line<std::string_view>(key_string_view);
    EXPECT_EQ(val_string_view, std::string_view(data.json["type"].asCString()));
    EXPECT_EQ(val_string_view, std::string_view(data.json["type"].asString()));
    std::tie(val_string_view) = reader.Line<std::string_view>(key_string);
    EXPECT_EQ(val_string_view, std::string_view(data.json["type"].asCString()));
    EXPECT_EQ(val_string_view, std::string_view(data.json["type"].asString()));

    std::tie(val_string) = reader.Line<std::string>(key_const_char_ptr);
    EXPECT_EQ(val_string, data.json["type"].asString());
    std::tie(val_string) = reader.Line<std::string>(key_string_view);
    EXPECT_EQ(val_string, data.json["type"].asString());
    std::tie(val_string) = reader.Line<std::string>(key_string);
    EXPECT_EQ(val_string, data.json["type"].asString());
}

TEST(JsonCppReaderTest, IntArray)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    const auto reader = json.CreateReader();

    std::vector<int> reader_value_vec;
    for (const auto& val : *reader.At("number_list").Raw())
    {
        reader_value_vec.push_back(val.asInt());
    }

    std::vector<int> json_value_vec;
    for (const auto& val : data.json["number_list"])
    {
        json_value_vec.push_back(val.asInt());
    }

    EXPECT_EQ(reader_value_vec, json_value_vec);
}

TEST(JsonCppReaderTest, StringArray)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    const auto reader = json.CreateReader();

    std::vector<std::string> reader_value_vec;
    for (const auto& val : *reader.At("string_list").Raw())
    {
        reader_value_vec.push_back(val.asString());
    }

    std::vector<std::string> json_value_vec;
    for (const auto& val : data.json["string_list"])
    {
        json_value_vec.push_back(val.asString());
    }

    EXPECT_EQ(reader_value_vec, json_value_vec);
}

TEST(JsonCppReaderTest, At)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    auto reader = json.CreateReader().At("list", 0);
    auto [name, price, is_cpp, is_rust] = reader.Line<std::string, double, bool, bool>(
        "name", "price", "is_cpp", "is_rust"
    );
    const Json::Value& obj0 = data.json["list"][0];
    EXPECT_EQ(name, obj0["name"].asString());
    EXPECT_EQ(price, obj0["price"].asDouble());
    EXPECT_EQ(is_cpp, obj0["is_cpp"].asBool());
    EXPECT_EQ(is_rust, obj0["is_rust"].asBool());

    reader = json.CreateReader().At("list", 1);
    std::tie(name, price, is_cpp, is_rust) = reader.Line<std::string, double, bool, bool>(
        "name", "price", "is_cpp", "is_rust"
    );
    const Json::Value& obj1 = data.json["list"][1];
    EXPECT_EQ(name, obj1["name"].asString());
    EXPECT_EQ(price, obj1["price"].asDouble());
    EXPECT_EQ(is_cpp, obj1["is_cpp"].asBool());
    EXPECT_EQ(is_rust, obj1["is_rust"].asBool());
}

TEST(JsonCppReaderTest, Move)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);

    auto reader = json.CreateReader();
    reader.Move("list", 0);
    auto [name, price, is_cpp, is_rust] = reader.Line<std::string, double, bool, bool>(
        "name", "price", "is_cpp", "is_rust"
    );
    const Json::Value& obj0 = data.json["list"][0];
    EXPECT_EQ(name, obj0["name"].asString());
    EXPECT_EQ(price, obj0["price"].asDouble());
    EXPECT_EQ(is_cpp, obj0["is_cpp"].asBool());
    EXPECT_EQ(is_rust, obj0["is_rust"].asBool());

    reader = json.CreateReader();
    reader.Move("list", 1);
    std::tie(name, price, is_cpp, is_rust) = reader.Line<std::string, double, bool, bool>(
        "name", "price", "is_cpp", "is_rust"
    );
    const Json::Value& obj1 = data.json["list"][1];
    EXPECT_EQ(name, obj1["name"].asString());
    EXPECT_EQ(price, obj1["price"].asDouble());
    EXPECT_EQ(is_cpp, obj1["is_cpp"].asBool());
    EXPECT_EQ(is_rust, obj1["is_rust"].asBool());
}

TEST(JsonCppReaderTest, ErrorMessage)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppReader json(&data.json);
    int64_t val_int = 0;
    std::string val_str;
    const char* val_ptr;
    std::string_view val_view;
    const std::string key = "not_exist_key";

    auto reader = json.CreateReader().SetErrorPolicy();
    reader.Move(key);
    std::tie(val_int) = reader.Line<int>(key);
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(reader.ErrorMessage(), std::format("cannot read key '{}' from null or non-object JSON", key));

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    reader.Move("number_list");
    std::tie(val_int) = reader.Line<int>(key);
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(reader.ErrorMessage(), std::format("cannot read key '{}' from null or non-object JSON", key));

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_int) = reader.Line<int>(key);
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(reader.ErrorMessage(), std::format("key '{}' not found in JSON", key));

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_ptr) = reader.Line<const char*>("user_number");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'user_number', expected const char*, got int")
    );

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_view) = reader.Line<std::string_view>("user_number");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'user_number', expected string_view, got int")
    );

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_str) = reader.Line<std::string>("user_number");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'user_number', expected string, got int")
    );

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_int) = reader.Line<int8_t>("type");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'type', expected int8_t, got string")
    );

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_int) = reader.Line<uint8_t>("type");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'type', expected uint8_t, got string")
    );

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_int) = reader.Line<int16_t>("type");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'type', expected short, got string")
    );

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_int) = reader.Line<uint16_t>("type");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'type', expected ushort, got string")
    );

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_int) = reader.Line<int32_t>("type");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'type', expected int, got string")
    );

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_int) = reader.Line<uint32_t>("type");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'type', expected uint, got string")
    );

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_int) = reader.Line<int64_t>("type");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'type', expected long, got string")
    );

    json.ClearError();
    reader = json.CreateReader().SetErrorPolicy();
    std::tie(val_int) = reader.Line<uint64_t>("type");
    EXPECT_TRUE(reader.Error());
    EXPECT_FALSE(reader.ErrorMessage().empty());
    EXPECT_EQ(
        reader.ErrorMessage(),
        std::string("type mismatched for key 'type', expected ulong, got string")
    );
}
