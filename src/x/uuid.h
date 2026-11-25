#pragma once

#include <string>

namespace x
{

struct Uuid
{
    bool lower_case = true;         ///< 小写形式
    bool remote_hyphen = false;     ///< 移除连字符
    [[nodiscard]] std::string Random() const;
};

}
