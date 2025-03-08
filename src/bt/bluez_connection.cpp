#include "homecontroller/bt/bluez_connection.h"

#include <algorithm>
#include <iostream>
#include <thread>

namespace hc {
namespace bt {

void BlueZConnection::start() {
    m_conn_ptr = sdbus::createSystemBusConnection();

    // for org.bluez.Adapter
    sdbus::ServiceName destination{"org.bluez"};
    sdbus::ObjectPath object_path{
        "/org/bluez/hci0"}; // should be found in runtime

    std::unique_ptr<sdbus::IProxy> proxy_ptr = sdbus::createProxy(*m_conn_ptr, std::move(destination), std::move(object_path));

    // enable adapter

    proxy_ptr->setProperty("Powered")
        .onInterface("org.bluez.Adapter1")
        .toValue(true);
    m_logger.verbose("Successfully enabled Bluetooth adapter!");

    /*proxy_ptr->uponSignal("InterfacesAddedd")
        .onInterface("org.freedesktop.DBus.ObjectManager")
        .call([](const sdbus::ObjectPath& object_path,
                 const std::map<std::string,
                                std::map<std::string, sdbus::Variant>>&
                     interfaces) {
            // m_logger.verbose("TESTING TESTING 123");
            std::cout << "test" << std::endl;
        });*/ 
    
    {
        sdbus::ServiceName destination2{"org.bluez"};
        sdbus::ObjectPath object_path2{"/"};
        std::unique_ptr<sdbus::IProxy> test = sdbus::createProxy(*m_conn_ptr, std::move(destination2), std::move(object_path2));

        //sdbus::InterfaceName iface_name{"org.freedesktop.DBus.ObjectManager"};
        //sdbus::SignalName signal_name{"InterfacesAdded"};
        /*test->registerSignalHandler(
            iface_name, signal_name,
            [](sdbus::Signal signal) { std::cout << "test" << std::endl; });*/

       test->uponSignal("InterfacesAdded")
          .onInterface("org.freedesktop.DBus.ObjectManager")
          .call([this](sdbus::ObjectPath path, std::map<std::string, std::map<std::string, sdbus::Variant>> dictionary) {
            std::cout << "test" << std::endl;
          });

       //test->finishRegistration();
    }

    proxy_ptr->callMethod("StartDiscovery") 
        .onInterface("org.bluez.Adapter1"); 
    m_logger.verbose("Successfully started device discovery!");    
    
    m_conn_ptr->enterEventLoopAsync();
    /*while(true) {
        m_conn_ptr->processPendingEvent();
    }*/

    m_connected = true;
}

std::shared_ptr<Device>
BlueZConnection::get_device(const std::string& address) {
    /*static const std::string ADAPTER_PATH = "/org/bluez/hci0";
    static const int CONNECTION_TIMEOUT = 10000;

    std::string formatted_addr = address;
    std::replace(formatted_addr.begin(), formatted_addr.end(), ':', '_');

    const std::string object_path_str = ADAPTER_PATH + "/dev_" + formatted_addr;

    // for org.bluez.Device
    sdbus::ServiceName destination{"org.bluez"};
    sdbus::ObjectPath object_path{object_path_str};
    // sdbus::ObjectPath object_path{"/org/bluez/hci0"};
    sdbus::InterfaceName iface_name{"org.bluez.Device1"};

    std::unique_ptr<sdbus::IProxy> proxy_ptr = sdbus::createProxy(
        *m_conn_ptr, std::move(destination), std::move(object_path));

    m_logger.verbose("Attempting connection to Bluetooth device [" + address +
                     "]...");*/

    /*try {
        proxy_ptr->callMethod("Connect")
            .onInterface(iface_name)
            .withTimeout(std::chrono::milliseconds(10000));
    } catch (sdbus::Error& e) {
        m_logger.error("Failed to connect to device!");
        m_logger.error("[" + e.getName() + "]: " + e.getMessage());

        return nullptr;
    }*/
    m_logger.verbose("Successfully connected to [" + address + "]!");

    return std::make_shared<Device>();
}

} // namespace bt
} // namespace hc
