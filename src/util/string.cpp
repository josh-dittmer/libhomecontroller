#include "homecontroller/util/string.h"

#include <algorithm>

namespace hc {
namespace util {
namespace str {

std::string to_lower_case(std::string str) {
    std::transform(
        str.begin(), str.end(), str.begin(),
        [](unsigned char c) -> unsigned char { return std::tolower(c); });

    return str;
}

} // namespace str
} // namespace util
} // namespace hc