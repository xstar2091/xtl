#include "x/uuid.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace x
{

std::string Uuid::Random() const
{
    std::string uuid = boost::uuids::to_string(boost::uuids::random_generator()());
    if (!with_hyphen) std::erase(uuid, '-');
    if (!lower_case) std::ranges::transform(uuid, uuid.begin(), [](const unsigned char c) { return std::toupper(c); });
    return uuid;
}

}
