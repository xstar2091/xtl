#include "utils/data/json/json_cpp_data.h"

#include <cstdio>
#include <fstream>

namespace x::test::data
{

JsonCppData::JsonCppData(const bool write_json_file)
{
    json_content = R"({
"type": "ide",
"day_use": false,
"month_use": true,
"user_number": 103,
"average_price": 1000.32,
"number_list": [17, 32, 96],
"string_list": ["apple", "banana", "orange"],
"list": [
    {
        "name": "clion",
        "price": 982.35,
        "is_cpp": true,
        "is_rust": false,
        "number_list_clion": [117, 132, 196],
        "string_list_clion": ["dog", "cat", "bird"],
    },
    {
        "name": "rust_rover",
        "price": 957.35,
        "is_cpp": false,
        "is_rust": true,
        "number_list_rust": [17, 32, 96],
        "string_list_rust": ["three", "one", "eight"],
    }
]
})";

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errs;
    reader->parse(
        json_content.c_str(),
        json_content.c_str() + json_content.length(),
        &json,
        &errs
    );

    if (write_json_file)
    {
        json_file_full_path = "/tmp/x_tests_json_cpp_data.json";
        std::ofstream ofs(json_file_full_path);
        ofs << json_content << std::endl;
    }
}

JsonCppData::~JsonCppData()
{
    if (!json_file_full_path.empty())
    {
        std::remove(json_file_full_path.c_str());
    }
}

}
