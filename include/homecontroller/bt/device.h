#pragma once

#include "homecontroller/util/logger.h"

#include <condition_variable>
#include <mutex>
#include <string>

#include <gattlib.h>

namespace hc {
namespace bt {

class Device {
  public:
    Device(gattlib_adapter_t* adapter, const std::string& address)
        : m_logger("BTDevice"), m_adapter(adapter), m_address(address) {}
    ~Device() {}

    bool connect();

  private:
    static void on_device_connect(gattlib_adapter_t* adapter,
                                  const char* dst_cstr,
                                  gattlib_connection_t* connection, int error,
                                  void* data);

    util::Logger m_logger;

    gattlib_adapter_t* m_adapter;
    std::string m_address;

    std::mutex m_mutex;
    std::condition_variable m_cv;
};

} // namespace bt
} // namespace hc