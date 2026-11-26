#pragma once

#include <jsoncpp/json/json.h>

namespace x::test::data
{

struct JsonCppData
{
    std::string json_file_full_path;
    std::string json_content;
    Json::Value json;
    explicit JsonCppData(bool write_json_file = false);
    ~JsonCppData();
};

}
