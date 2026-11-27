#pragma once

#include <jsoncpp/json/json.h>

namespace x::data
{

class JsonCppLoader
{
    Json::Value root_;
    std::string error_message_;

public:
    JsonCppLoader& FromFile(std::string_view file_path);
    JsonCppLoader& FromString(std::string_view json_string);
    JsonCppLoader& ExpandEnvVars();

    void clear_error_message() { error_message_.clear(); }
    [[nodiscard]] bool error() const { return !error_message_.empty(); }
    [[nodiscard]] const std::string& error_message() const { return error_message_; }
    [[nodiscard]] const Json::Value& json() const { return root_; }

private:
    void ExpandRecursive(Json::Value& root);
    void ExpandStringValue(std::string_view key, Json::Value& root);
    static bool ExpandPlaceholder(std::string_view type, std::string_view value, Json::Value& obj);
};

}
