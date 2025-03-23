#include "homecontroller/util/logger.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace hc {
namespace util {
std::string Logger::_main_prefix = "[HC]";

Logger::LogLevel Logger::_log_level = Logger::LogLevel::NONE;

// test

std::map<Logger::LogType, Logger::LogTypeInfo> Logger::_log_type_info_table = {
    {Logger::LogType::LOG, {"[LOG]", "37"}},
    {Logger::LogType::FATAL, {"[FATAL]", "91"}},
    {Logger::LogType::ERROR, {"[ERROR]", "31"}},
    {Logger::LogType::WARN, {"[WARN]", "33"}},
    {Logger::LogType::DEBUG, {"[DEBUG]", "90"}},
    {Logger::LogType::VERBOSE, {"[VERBOSE]", "90"}}};

const std::string Logger::_log_level_none_str = "NONE";
const std::string Logger::_log_level_normal_str = "NORMAL";
const std::string Logger::_log_level_debug_str = "DEBUG";
const std::string Logger::_log_level_verbose_str = "VERBOSE";

Logger::Logger(std::string context) {
    m_context_str = (context.length() > 11) ? context.substr(0, 11) : context;
}

void Logger::set_log_level(Logger::LogLevel level) { _log_level = level; }

Logger::LogLevel Logger::string_to_log_level(const Logger& logger,
                                             const std::string& str) {
    static std::map<std::string, LogLevel> str_to_log_level_map = {
        {_log_level_none_str, LogLevel::NONE},
        {_log_level_normal_str, LogLevel::NORMAL},
        {_log_level_debug_str, LogLevel::DEBUG},
        {_log_level_verbose_str, LogLevel::VERBOSE}};

    auto mit = str_to_log_level_map.find(str);
    if (mit == str_to_log_level_map.end()) {
        logger.warn(
            "Unknown log level specified, defaulting to LogLevel::NORMAL");
        return LogLevel::NORMAL;
    }

    return mit->second;
}

void Logger::log(const std::string& msg, bool nl) const {
    static LogTypeInfo info = _log_type_info_table[LogType::LOG];
    if (_log_level >= LogLevel::NORMAL)
        print(info.prefix, msg, nl, info.color);
}

void Logger::fatal(const std::string& msg, bool nl) const {
    static LogTypeInfo info = _log_type_info_table[LogType::FATAL];
    if (_log_level >= LogLevel::NONE)
        print(info.prefix, msg, nl, info.color);
}

void Logger::error(const std::string& msg, bool nl) const {
    static LogTypeInfo info = _log_type_info_table[LogType::ERROR];
    if (_log_level >= LogLevel::NORMAL)
        print(info.prefix, msg, nl, info.color);
}

void Logger::warn(const std::string& msg, bool nl) const {
    static LogTypeInfo info = _log_type_info_table[LogType::WARN];
    if (_log_level >= LogLevel::NORMAL)
        print(info.prefix, msg, nl, info.color);
}

void Logger::debug(const std::string& msg, bool nl) const {
    static LogTypeInfo info = _log_type_info_table[LogType::DEBUG];
    if (_log_level >= LogLevel::DEBUG)
        print(info.prefix, msg, nl, info.color);
}

void Logger::verbose(const std::string& msg, bool nl) const {
    static LogTypeInfo info = _log_type_info_table[LogType::VERBOSE];
    if (_log_level >= LogLevel::VERBOSE)
        print(info.prefix, msg, nl, info.color);
}

void Logger::print(const std::string& prefix, const std::string& msg, bool nl,
                   const std::string& color) const {
    const std::string spaces1 = std::string(10 - prefix.length(), ' ');
    const std::string spaces2 = std::string(15 - m_context_str.length(), ' ');

    std::ostringstream ss;

    ss << "\x1b[" << color << "m" << _main_prefix << " " << timestamp() << " "
       << prefix << spaces1 << "[" << m_context_str << "]" << spaces2 << msg
       << "\x1b[0m";

    if (nl)
        ss << "\n";

    std::cout << ss.str();
}

std::string Logger::timestamp() const {
    auto t = std::time(nullptr);

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t), "%m-%d-%y %H:%M:%S");

    return ss.str();
}
} // namespace util
} // namespace hc