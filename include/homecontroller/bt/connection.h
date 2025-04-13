#pragma once

#include "homecontroller/util/logger.h"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#define GATTLIB_LOG_LEVEL 3
#include <gattlib.h>

namespace hc {
namespace bt {
class Connection {
  public:
    Connection(std::mutex& mutex_ref, gattlib_adapter_t* adapter,
               const std::string& address, const std::string& name)
        : m_logger("BTConnection"), m_adapter(adapter), m_address(address),
          m_name(name), m_mutex_ref(mutex_ref), m_should_exit(false),
          m_running(false) {}
    ~Connection() {}

    void start();
    void shutdown();
    void await_finish();

    void write_char(int test);

  private:
    static void on_device_connect(gattlib_adapter_t* adapter,
                                  const char* dst_cstr,
                                  gattlib_connection_t* connection, int error,
                                  void* data);

    static void on_device_disconnect(gattlib_connection_t* connection,
                                     void* data);

    void loop_thread();
    void perform_discovery(gattlib_connection_t* connection);

    util::Logger m_logger;

    gattlib_adapter_t* m_adapter;
    std::string m_address;
    std::string m_name;

    std::thread m_loop_thread;

    std::queue<int> m_queue;

    std::mutex& m_mutex_ref;
    std::condition_variable m_cv_finished;
    std::condition_variable m_cv_queue;

    bool m_should_exit;
    bool m_running;
};
} // namespace bt
} // namespace hc