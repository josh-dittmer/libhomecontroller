#include "homecontroller/bt/scanner.h"

namespace hc {
namespace bt {

bool Scanner::start(const std::set<std::string>& addresses) {
    m_logger.verbose("start(): Starting scan loop...");

    m_addresses = addresses;

    m_loop_thread = std::thread(&Scanner::loop_thread, this);

    return true;
}

void Scanner::shutdown() {
    for (auto& c : m_connections) {
        c.second->shutdown();
    }
}

void Scanner::await_finish_and_cleanup() {
    if (m_loop_thread.joinable()) {
        m_loop_thread.join();
    }

    /*std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& c : m_connections) {
        c.second->shutdown();
    }

    m_mutex.unlock();

    if (m_adapter != nullptr) {
        std::cout << "." << std::endl;
        gattlib_adapter_scan_disable(m_adapter);

        m_logger.verbose("shutdown(): Waiting for scan to stop...");

        if (m_loop_thread.joinable()) {
            m_loop_thread.join();
        }
    }

    /*std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& c : m_connections) {
        c.second->shutdown();
    }
    std::cout << ".." << std::endl;

    if (m_adapter != nullptr) {
        std::cout << "..." << std::endl;
        gattlib_adapter_scan_disable(m_adapter);
        std::cout << "...." << std::endl;
    }

    if (m_loop_thread.joinable()) {
        m_loop_thread.join();
    }
    std::cout << "....." << std::endl;*/
}

std::shared_ptr<Connection>
Scanner::get_connection(const std::string& address) {
    /*std::cout << "get_connection enter" << std::endl;
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "get_connection exit" << std::endl;

    auto mit = m_connections.find(address);
    if (mit == m_connections.end()) {
        return nullptr;
    }

    return mit->second;*/
}

void Scanner::loop_thread() {
    int ret = gattlib_mainloop(Scanner::scan_task, this);

    if (ret != GATTLIB_SUCCESS) {
        m_logger.verbose("start(): Scan loop exited with error!");
        return;
    } else {
        m_logger.verbose("start(): Scan loop exited with no errors");
    }
}

void* Scanner::scan_task(void* data) {
    static const int TIMEOUT = 30;

    Scanner* instance = reinterpret_cast<Scanner*>(data);

    if (gattlib_adapter_open(nullptr, &instance->m_adapter)) {
        instance->m_logger.error("Failed to open adapter!");
        return nullptr;
    }

    instance->m_logger.log("Scan enabled (" + std::to_string(TIMEOUT) +
                           "s) for the following addresses:");
    for (auto& addr : instance->m_addresses) {
        instance->m_logger.log("[" + addr + "]");
    }

    if (gattlib_adapter_scan_enable(
            instance->m_adapter, Scanner::on_device_discovered, 30, instance)) {
        instance->m_logger.error("Failed to start scan!");
        return nullptr;
    }

    gattlib_adapter_scan_disable(instance->m_adapter);

    instance->m_logger.log("Scan complete!");

    for (auto& c : instance->m_connections) {
        c.second->start();
    }

    for (auto& c : instance->m_connections) {
        c.second->await_finish_and_cleanup();
    }

    return nullptr;
}

void Scanner::on_device_discovered(gattlib_adapter_t* adapter,
                                   const char* addr_cstr, const char* name_cstr,
                                   void* data) {
    Scanner* instance = reinterpret_cast<Scanner*>(data);

    if (addr_cstr == nullptr) {
        return;
    }

    std::string address(addr_cstr);
    std::string name(name_cstr != nullptr ? name_cstr : "???");

    instance->m_logger.log(address);

    if (instance->m_addresses.find(address) != instance->m_addresses.end()) {
        instance->m_logger.log("Found device: " + name + " @ [" + address +
                               "]");

        instance->create_connection(adapter, address);

        if (instance->m_connections.size() >= instance->m_addresses.size()) {
            gattlib_adapter_scan_disable(instance->m_adapter);
        }
    }
}

void Scanner::create_connection(gattlib_adapter_t* adapter,
                                const std::string& address) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::shared_ptr<Connection> conn_ptr =
        std::make_shared<Connection>(adapter, address);

    m_connections.insert(std::make_pair(address, conn_ptr));
}

} // namespace bt
} // namespace hc