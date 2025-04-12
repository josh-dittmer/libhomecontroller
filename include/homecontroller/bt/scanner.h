#pragma once

#include "homecontroller/bt/connection.h"
#include "homecontroller/util/logger.h"

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

#include <gattlib.h>

namespace hc {
namespace bt {

class Scanner {
  public:
    Scanner() : m_logger("BTScanner") {}
    ~Scanner() {}

    bool start(const std::set<std::string>& addresses);
    void shutdown();
    void await_finish_and_cleanup();

    std::shared_ptr<Connection> get_connection(const std::string& address);

  private:
    void loop_thread();

    static void* scan_task(void* data);
    static void on_device_discovered(gattlib_adapter_t* adapter,
                                     const char* addr_cstr,
                                     const char* name_cstr, void* data);

    void create_connection(gattlib_adapter_t* adapter,
                           const std::string& address);

    util::Logger m_logger;

    std::thread m_loop_thread;

    gattlib_adapter_t* m_adapter;

    std::set<std::string> m_addresses;
    std::map<std::string, std::shared_ptr<Connection>> m_connections;

    std::mutex m_mutex;
    std::condition_variable m_cv;
};

} // namespace bt
} // namespace hc