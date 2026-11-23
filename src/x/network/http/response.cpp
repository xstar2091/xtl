#include "x/network/http/response.h"

namespace x::http
{

int Response::StatusCode() const
{
    return std::visit([]<typename T>(const T& resp) -> int {
        if constexpr (std::is_same_v<T, std::monostate>)
        {
            return 0;
        }
        else
        {
            return static_cast<int>(resp.result_int());
        }
    }, response_);
}

}
