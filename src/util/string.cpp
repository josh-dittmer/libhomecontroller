#include "homecontroller/util/string.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace hc {
namespace util {
namespace str {

std::string to_lower_case(std::string str) {
    std::transform(
        str.begin(), str.end(), str.begin(),
        [](unsigned char c) -> unsigned char { return std::tolower(c); });

    return str;
}

std::string to_hex(int num, int width) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::setfill('0') << std::setw(width) << num;
    return ss.str();
}

} // namespace str
} // namespace util
} // namespace hc