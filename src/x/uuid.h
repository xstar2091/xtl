#pragma once

#include <string>

namespace x
{

struct Uuid
{
    bool lower_case = true;         ///< 小写形式
    bool with_hyphen = true;        ///< 保留连字符
    [[nodiscard]] std::string Random() const;
};

}
