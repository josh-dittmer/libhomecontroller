#include "homecontroller/bt/bluez_connection.h"

#include <algorithm>

namespace hc {
namespace bt {

void BlueZConnection::start() {
    m_conn_ptr = sdbus::createSystemBusConnection();

    // for org.bluez.Adapter
    sdbus::ServiceName destination{"org.bluez"};
    sdbus::ObjectPath object_path{
        "/org/bluez/hci0"}; // should be found in runtime
    sdbus::InterfaceName iface_name{"org.bluez.Adapter1"};

    std::unique_ptr<sdbus::IProxy> proxy_ptr = sdbus::createProxy(
        *m_conn_ptr, std::move(destination), std::move(object_path));

    // enable adapter
    proxy_ptr->setProperty("Powered").onInterface(iface_name).toValue(true);

    m_connected = true;

    m_logger.verbose("Successfully enabled Bluetooth adapter!");
}

std::shared_ptr<Device>
BlueZConnection::get_device(const std::string& address) {
    static const std::string ADAPTER_PATH = "/org/bluez/hci0";
    static const int CONNECTION_TIMEOUT = 10000;

    std::string formatted_addr = address;
    std::replace(formatted_addr.begin(), formatted_addr.end(), ':', '_');

    const std::string object_path_str = ADAPTER_PATH + "/dev_" + formatted_addr;

    // for org.bluez.Device
    sdbus::ServiceName destination{"org.bluez"};
    sdbus::ObjectPath object_path{object_path_str};
    sdbus::InterfaceName iface_name{"org.bluez.Device1"};

    std::unique_ptr<sdbus::IProxy> proxy_ptr = sdbus::createProxy(
        *m_conn_ptr, std::move(destination), std::move(object_path));

    m_logger.verbose("Attempting connection to Bluetooth device [" + address +
                     "]...");

    proxy_ptr->callMethod("Connect")
        .onInterface(iface_name)
        .withTimeout(std::chrono::milliseconds(10000));

    m_logger.verbose("Successfully connected to [" + address + "]!");

    return std::make_shared<Device>();
}

} // namespace bt
} // namespace hc
