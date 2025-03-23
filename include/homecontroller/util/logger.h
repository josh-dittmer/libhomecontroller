#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace hc {
namespace util {
class Logger {
  public:
    Logger(std::string context);

    ~Logger() {}

    void log(const std::string& msg, bool nl = true) const;
    void fatal(const std::string& msg, bool nl = true) const;
    void error(const std::string& msg, bool nl = true) const;
    void warn(const std::string& msg, bool nl = true) const;
    void debug(const std::string& msg, bool nl = true) const;
    void verbose(const std::string& msg, bool nl = true) const;

    enum class LogLevel { NONE, NORMAL, DEBUG, VERBOSE };

    static void set_log_level(LogLevel level);
    static LogLevel string_to_log_level(const Logger& logger,
                                        const std::string& str);

  private:
    void print(const std::string& prefix, const std::string& msg, bool nl,
               const std::string& color) const;
    std::string timestamp() const;

    std::string m_context_str;

    enum class LogType : uint8_t {
        LOG = 0,
        FATAL = 1,
        ERROR = 2,
        WARN = 3,
        DEBUG = 4,
        VERBOSE = 5
    };

    struct LogTypeInfo {
        std::string prefix;
        std::string color;
    };

    static std::string _main_prefix;

    static LogLevel _log_level;
    static std::map<LogType, LogTypeInfo> _log_type_info_table;

    static const std::string _log_level_none_str;
    static const std::string _log_level_normal_str;
    static const std::string _log_level_debug_str;
    static const std::string _log_level_verbose_str;
};
} // namespace util
} // namespace hc