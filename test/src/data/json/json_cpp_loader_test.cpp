#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include "x/data/json/json_cpp_loader.h"
#include "utils/data/json/json_cpp_data.h"
#include "x/data/json/json_cpp_reader.h"

TEST(JsonCppLoaderTest, FromFile_NotExist)
{
    x::data::JsonCppLoader loader;
    loader.FromFile("/tmp/not_exist.json");
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    EXPECT_EQ(loader.error_message(), "open json file failed");
}

TEST(JsonCppLoaderTest, FromFile_InvalidJson)
{
    constexpr auto file_path = "/tmp/invalid_json_file.json";
    std::ofstream ofs(file_path);
    ofs << "abc" << std::endl;

    x::data::JsonCppLoader loader;
    loader.FromFile(file_path);
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());

    std::remove(file_path);
}

TEST(JsonCppLoaderTest, FromFile)
{
    const x::test::data::JsonCppData data(true);
    x::data::JsonCppLoader loader;
    loader.FromFile(data.json_file_full_path);

    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());

    x::data::JsonCppReader::Reader reader(&loader.json(), const_cast<std::string*>(&loader.error_message()));
    reader.SetErrorPolicy();

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

    reader.Move("list", 0);
    auto [name, price, is_cpp, is_rust] = reader.Line<std::string, double, bool, bool>(
        "name", "price", "is_cpp", "is_rust"
    );
    const Json::Value& obj0 = data.json["list"][0];
    EXPECT_EQ(name, obj0["name"].asString());
    EXPECT_EQ(price, obj0["price"].asDouble());
    EXPECT_EQ(is_cpp, obj0["is_cpp"].asBool());
    EXPECT_EQ(is_rust, obj0["is_rust"].asBool());
}

TEST(JsonCppLoaderTest, FromString_InvalidJson)
{
    x::data::JsonCppLoader loader;
    loader.FromString("abc");
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
}

TEST(JsonCppLoaderTest, FromString)
{
    const x::test::data::JsonCppData data;
    x::data::JsonCppLoader loader;
    loader.FromString(data.json_content);

    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());

    x::data::JsonCppReader::Reader reader(&loader.json(), const_cast<std::string*>(&loader.error_message()));
    reader.SetErrorPolicy();

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

    reader.Move("list", 0);
    auto [name, price, is_cpp, is_rust] = reader.Line<std::string, double, bool, bool>(
        "name", "price", "is_cpp", "is_rust"
    );
    const Json::Value& obj0 = data.json["list"][0];
    EXPECT_EQ(name, obj0["name"].asString());
    EXPECT_EQ(price, obj0["price"].asDouble());
    EXPECT_EQ(is_cpp, obj0["is_cpp"].asBool());
    EXPECT_EQ(is_rust, obj0["is_rust"].asBool());
}

TEST(JsonCppLoaderTest, ExpandEnvVars_String_Env)
{
    auto* env_key = "ENV_NAME";
    auto* env_val = "lucy";
    auto* json_content = R"({
    "name": "${ENV_NAME}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    std::string json_value_after_expanded = std::format("${{{}}}", env_key);
    json_value_after_expanded.back() = '\0';
    EXPECT_EQ(loader.json()["name"].asString(), json_value_after_expanded);
    EXPECT_EQ(
        loader.error_message(),
        "read value from env failed, env var not found and no default value set for json key 'name'"
    );

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["name"].asString(), std::string(env_val));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_String_EnvType)
{
    auto* env_key = "ENV_NAME";
    auto* env_val = "lucy";
    auto* json_content = R"({
    "name": "${ENV_NAME:string}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    std::string json_value_after_expanded = std::format("${{{}:string}}", env_key);
    json_value_after_expanded.back() = '\0';
    json_value_after_expanded[json_value_after_expanded.find(':')] = '\0';
    EXPECT_EQ(loader.json()["name"].asString(), json_value_after_expanded);
    EXPECT_EQ(
        loader.error_message(),
        "read value from env failed, env var not found and no default value set for json key 'name'"
    );

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["name"].asString(), std::string(env_val));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_String_EnvTypeDefault)
{
    auto* env_key = "ENV_NAME";
    auto* env_val = "lucy";
    auto* json_content = R"({
    "name": "${ENV_NAME:string:lily}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["name"].asString(), std::string("lily"));

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["name"].asString(), std::string(env_val));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_Bool_EnvType)
{
    auto* env_key = "ENV_IS_SUNDAY";
    auto* env_val_1 = "true";
    auto* env_val_2 = "false";
    auto* env_val_3 = "abc";
    auto* json_content = R"({
    "is_sunday": "${ENV_IS_SUNDAY:bool}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    std::string json_value_after_expanded = std::format("${{{}:bool}}", env_key);
    json_value_after_expanded.back() = '\0';
    json_value_after_expanded[json_value_after_expanded.find(':')] = '\0';
    EXPECT_EQ(loader.json()["is_sunday"].asString(), json_value_after_expanded);
    EXPECT_EQ(
        loader.error_message(),
        "read value from env failed, env var not found and no default value set for json key 'is_sunday'"
    );

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_1, "true") == 0));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_2, "true") == 0));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_3, "true") == 0));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_Bool_EnvTypeDefaultTrue)
{
    auto* env_key = "ENV_IS_SUNDAY";
    auto* env_val_1 = "true";
    auto* env_val_2 = "false";
    auto* env_val_3 = "abc";
    auto* json_content = R"({
    "is_sunday": "${ENV_IS_SUNDAY:bool:true}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_TRUE(loader.json()["is_sunday"].asBool());

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_1, "true") == 0));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_2, "true") == 0));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_3, "true") == 0));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_Bool_EnvTypeDefaultFalse)
{
    auto* env_key = "ENV_IS_SUNDAY";
    auto* env_val_1 = "true";
    auto* env_val_2 = "false";
    auto* env_val_3 = "abc";
    auto* json_content = R"({
    "is_sunday": "${ENV_IS_SUNDAY:bool:false}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_FALSE(loader.json()["is_sunday"].asBool());

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_1, "true") == 0));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_2, "true") == 0));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_3, "true") == 0));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_Bool_EnvTypeDefaultNotBool)
{
    auto* env_key = "ENV_IS_SUNDAY";
    auto* env_val_1 = "true";
    auto* env_val_2 = "false";
    auto* env_val_3 = "abc";
    auto* json_content = R"({
    "is_sunday": "${ENV_IS_SUNDAY:bool:NotBool}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_FALSE(loader.json()["is_sunday"].asBool());

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_1, "true") == 0));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_2, "true") == 0));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["is_sunday"].asBool(), (strcmp(env_val_3, "true") == 0));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_INT_EnvType)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:int}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    std::string json_value_after_expanded = std::format("${{{}:int}}", env_key);
    json_value_after_expanded.back() = '\0';
    json_value_after_expanded[json_value_after_expanded.find(':')] = '\0';
    EXPECT_EQ(loader.json()["age"].asString(), json_value_after_expanded);
    EXPECT_EQ(
        loader.error_message(),
        "read value from env failed, env var not found and no default value set for json key 'age'"
    );

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_INT_EnvTypeDefault17)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:int:17}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), 17);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_INT_EnvTypeDefault0)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:int:0}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), 0);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_INT_EnvTypeDefault36)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:int:36}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), 36);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_INT_EnvTypeDefaultNegative17)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:int:-17}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), -17);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_INT_EnvTypeDefaultNegative36)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:int:-36}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), -36);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asInt(), strtol(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_UINT_EnvType)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:uint}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    std::string json_value_after_expanded = std::format("${{{}:uint}}", env_key);
    json_value_after_expanded.back() = '\0';
    json_value_after_expanded[json_value_after_expanded.find(':')] = '\0';
    EXPECT_EQ(loader.json()["age"].asString(), json_value_after_expanded);
    EXPECT_EQ(
        loader.error_message(),
        "read value from env failed, env var not found and no default value set for json key 'age'"
    );

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_UINT_EnvTypeDefault17)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:uint:17}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), 17);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_UINT_EnvTypeDefault0)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:uint:0}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), 0);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_UINT_EnvTypeDefault36)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:uint:36}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), 36);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_UINT_EnvTypeDefaultNegative17)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:uint:-17}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), -17);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_UINT_EnvTypeDefaultNegative36)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36";
    auto* env_val_4 = "-17";
    auto* env_val_5 = "-36";
    auto* json_content = R"({
    "age": "${ENV_AGE:uint:-36}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), -36);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_1, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_2, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_3, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_4, nullptr, 10));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asUInt64(), strtoul(env_val_5, nullptr, 10));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_DOUBLE_EnvType)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17.12345";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36.12345";
    auto* env_val_4 = "-17.12345";
    auto* env_val_5 = "-36.12345";
    auto* json_content = R"({
    "age": "${ENV_AGE:double}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    std::string json_value_after_expanded = std::format("${{{}:double}}", env_key);
    json_value_after_expanded.back() = '\0';
    json_value_after_expanded[json_value_after_expanded.find(':')] = '\0';
    EXPECT_EQ(loader.json()["age"].asString(), json_value_after_expanded);
    EXPECT_EQ(
        loader.error_message(),
        "read value from env failed, env var not found and no default value set for json key 'age'"
    );

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_1, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_2, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_3, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_4, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_5, nullptr));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_DOUBLE_EnvTypeDefault17)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17.12345";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36.12345";
    auto* env_val_4 = "-17.12345";
    auto* env_val_5 = "-36.12345";
    auto* json_content = R"({
    "age": "${ENV_AGE:double:17.12345}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), 17.12345);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_1, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_2, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_3, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_4, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_5, nullptr));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_DOUBLE_EnvTypeDefault0)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17.12345";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36.12345";
    auto* env_val_4 = "-17.12345";
    auto* env_val_5 = "-36.12345";
    auto* json_content = R"({
    "age": "${ENV_AGE:double:0}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), 0);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_1, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_2, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_3, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_4, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_5, nullptr));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_DOUBLE_EnvTypeDefault36)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17.12345";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36.12345";
    auto* env_val_4 = "-17.12345";
    auto* env_val_5 = "-36.12345";
    auto* json_content = R"({
    "age": "${ENV_AGE:double:36.12345}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), 36.12345);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_1, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_2, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_3, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_4, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_5, nullptr));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_DOUBLE_EnvTypeDefaultNegative17)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17.12345";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36.12345";
    auto* env_val_4 = "-17.12345";
    auto* env_val_5 = "-36.12345";
    auto* json_content = R"({
    "age": "${ENV_AGE:double:-17.12345}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), -17.12345);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_1, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_2, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_3, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_4, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_5, nullptr));
    unsetenv(env_key);
}

TEST(JsonCppLoaderTest, ExpandEnvVars_DOUBLE_EnvTypeDefaultNegative36)
{
    auto* env_key = "ENV_AGE";
    auto* env_val_1 = "17.12345";
    auto* env_val_2 = "0";
    auto* env_val_3 = "36.12345";
    auto* env_val_4 = "-17.12345";
    auto* env_val_5 = "-36.12345";
    auto* json_content = R"({
    "age": "${ENV_AGE:double:-36.12345}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), -36.12345);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_1, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_1, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_2, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_2, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_3, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_3, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_4, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_4, nullptr));
    unsetenv(env_key);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val_5, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_DOUBLE_EQ(loader.json()["age"].asDouble(), strtod(env_val_5, nullptr));
    unsetenv(env_key);
}

TEST(JsonCppLoader, ExpandEnvVars_String_EmptyString)
{
    auto* env_key = "ENV_AGE";
    auto* env_val = "";
    auto* json_content = R"({
    "age": "${ENV_AGE:string:}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asString(), std::string());

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asString(), std::string());
    unsetenv(env_key);
}

TEST(JsonCppLoader, ExpandEnvVars_Bool_EmptyString)
{
    auto* env_key = "ENV_AGE";
    auto* env_val = "";
    auto* json_content = R"({
    "age": "${ENV_AGE:bool:}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asBool(), false);

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val, 1);
    loader.ExpandEnvVars();
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    EXPECT_EQ(loader.json()["age"].asBool(), false);
    unsetenv(env_key);
}

TEST(JsonCppLoader, ExpandEnvVars_Int_EmptyString)
{
    auto* env_key = "ENV_AGE";
    auto* env_val = "";
    auto* json_content = R"({
    "age": "${ENV_AGE:int:}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    EXPECT_EQ(
        loader.error_message(),
        "parse value '' to int failed for key 'age'"
    );

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val, 1);
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    EXPECT_EQ(
        loader.error_message(),
        "parse value '' to int failed for key 'age'"
    );
    unsetenv(env_key);
}

TEST(JsonCppLoader, ExpandEnvVars_UInt_EmptyString)
{
    auto* env_key = "ENV_AGE";
    auto* env_val = "";
    auto* json_content = R"({
    "age": "${ENV_AGE:uint:}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    EXPECT_EQ(
        loader.error_message(),
        "parse value '' to uint failed for key 'age'"
    );

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val, 1);
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    EXPECT_EQ(
        loader.error_message(),
        "parse value '' to uint failed for key 'age'"
    );
    unsetenv(env_key);
}

TEST(JsonCppLoader, ExpandEnvVars_Double_EmptyString)
{
    auto* env_key = "ENV_AGE";
    auto* env_val = "";
    auto* json_content = R"({
    "age": "${ENV_AGE:double:}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    EXPECT_EQ(
        loader.error_message(),
        "parse value '' to double failed for key 'age'"
    );

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val, 1);
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    EXPECT_EQ(
        loader.error_message(),
        "parse value '' to double failed for key 'age'"
    );
    unsetenv(env_key);
}

TEST(JsonCppLoader, ExpandEnvVars_UnknownType_EmptyString)
{
    auto* env_key = "ENV_AGE";
    auto* env_val = "";
    auto* json_content = R"({
    "age": "${ENV_AGE:unknown_type:}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    EXPECT_EQ(
        loader.error_message(),
        "parse value '' to unknown_type failed for key 'age'"
    );

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    setenv(env_key, env_val, 1);
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    EXPECT_EQ(
        loader.error_message(),
        "parse value '' to unknown_type failed for key 'age'"
    );
    unsetenv(env_key);
}

TEST(JsonCppLoader, UnknownType)
{
    auto* json_content = R"({
    "age": "${ENV_AGE:unknown:-36.12345}"
})";

    x::data::JsonCppLoader loader;

    loader.FromString(json_content);
    EXPECT_FALSE(loader.error());
    EXPECT_TRUE(loader.error_message().empty());
    loader.ExpandEnvVars();
    EXPECT_TRUE(loader.error());
    EXPECT_FALSE(loader.error_message().empty());
    EXPECT_EQ(
        loader.error_message(),
        "parse value '-36.12345' to unknown failed for key 'age'"
    );
}
