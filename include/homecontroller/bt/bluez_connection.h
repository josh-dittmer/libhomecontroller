#pragma once

#include "homecontroller/bt/device.h"
#include "homecontroller/util/logger.h"

#include <memory>

#include <sdbus-c++/sdbus-c++.h>

namespace hc {
namespace bt {

class BlueZConnection {
  public:
    BlueZConnection() : m_logger("BlueZ"), m_connected(false) {}
    ~BlueZConnection() {}

    void start();

    std::shared_ptr<Device> get_device(const std::string& address);

  private:
    util::Logger m_logger;

    std::unique_ptr<sdbus::IConnection> m_conn_ptr;
    std::unique_ptr<sdbus::IProxy> m_proxy_ptr;

    bool m_connected;
};

} // namespace bt
} // namespace hc