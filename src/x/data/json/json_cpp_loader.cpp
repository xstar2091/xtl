#include "x/data/json/json_cpp_loader.h"

#include <fstream>

namespace x::data
{

JsonCppLoader& JsonCppLoader::FromFile(const std::string_view file_path)
{
    std::ifstream ifs(file_path.data());
    if (!ifs.is_open())
    {
        error_message_ = "open json file failed";
        return *this;
    }

    Json::CharReaderBuilder builder;
    Json::parseFromStream(builder, ifs, &root_, &error_message_);
    return *this;
}

JsonCppLoader& JsonCppLoader::FromString(const std::string_view json_string)
{
    const Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> json_reader(builder.newCharReader());
    json_reader->parse(json_string.data(), json_string.data() + json_string.size(), &root_, &error_message_);
    return *this;
}

JsonCppLoader& JsonCppLoader::ExpandEnvVars()
{
    if (error()) return *this;
    if (root_.isObject() || root_.isArray())
    {
        ExpandRecursive(root_);
    }
    else
    {
        error_message_ = "json is not object or array";
    }
    return *this;
}

void JsonCppLoader::ExpandRecursive(Json::Value& root)
{
    for (auto it = root.begin(); it != root.end(); ++it)
    {
        if (error()) break;

        switch (Json::Value& value = *it; value.type())
        {
        case Json::nullValue:
        case Json::intValue:
        case Json::uintValue:
        case Json::realValue:
        case Json::booleanValue:
            break;
        case Json::arrayValue:
        case Json::objectValue:
            ExpandRecursive(value);
            break;
        case Json::stringValue:
            ExpandStringValue(it.key().asCString(), value);
            break;
        }
    }
}

void JsonCppLoader::ExpandStringValue(const std::string_view key, Json::Value& root)
{
    if (error()) return;
    std::string_view value_view = root.asCString();
    if (value_view.size() < 4) return;
    if (!value_view.starts_with("${") || !value_view.ends_with('}')) return;
    const_cast<std::string_view::reference>(value_view.back()) = '\0';
    value_view = std::string_view(value_view.data() + 2, value_view.size() - 3);
    std::vector<std::string_view> tokens;
    if (std::string_view::size_type pos = value_view.find(':'); pos != std::string_view::npos)
    {
        const_cast<std::string_view::reference>(value_view[pos]) = '\0';
        tokens.emplace_back(value_view.data(), pos);
        value_view = std::string_view(value_view.data() + pos + 1, value_view.size() - pos - 1);
        pos = value_view.find(':');
        if (pos != std::string_view::npos)
        {
            const_cast<std::string_view::reference>(value_view[pos]) = '\0';
            tokens.emplace_back(value_view.data(), pos);
            value_view = std::string_view(value_view.data() + pos + 1, value_view.size() - pos - 1);
            tokens.push_back(value_view);
        }
        else
        {
            tokens.push_back(value_view);
        }
    }
    else
    {
        tokens.push_back(value_view);
        tokens.emplace_back("string");
    }
    if (tokens.empty()) return;

    const char* value_ptr = std::getenv(std::string(tokens[0]).c_str());
    if (value_ptr == nullptr)
    {
        if (tokens.size() == 3)
        {
            value_ptr = tokens[2].data();
        }
    }
    if (value_ptr == nullptr)
    {
        error_message_ = std::format(
            "read value from env failed, env var not found and no default value set for json key '{}'",
            key
        );
        return;
    }
    if (!ExpandPlaceholder(tokens[1], value_ptr, root))
    {
        errno = 0;
        error_message_ = std::format("parse value '{}' to {} failed for key '{}'", value_ptr, tokens[1], key);
    }
}

bool JsonCppLoader::ExpandPlaceholder(
    const std::string_view type,
    const std::string_view value,
    Json::Value& obj
)
{
    if (type == "bool")
    {
        obj = value == "true";
        return true;
    }
    if (type == "string")
    {
        obj = value;
        return true;
    }
    if (value.empty())
    {
        errno = EINVAL;
        return false;
    }

    errno = 0;
    char* end_ptr = nullptr;
    if (type == "int")
    {
        if (const long int val = strtol(value.data(), &end_ptr, 10); errno == 0 && end_ptr[0] == '0')
        {
            obj = val;
        }
    }
    else if (type == "uint")
    {
        if (const unsigned long int val = strtoul(value.data(), &end_ptr, 10); errno == 0 && end_ptr[0] == '\0')
        {
            obj = val;
        }
    }
    else if (type == "double")
    {
        if (const double val = strtod(value.data(), &end_ptr); errno == 0 && end_ptr[0] == '\0')
        {
            obj = val;
        }
    }
    else
    {
        errno = EINVAL;
    }
    return errno == 0;
}

}
