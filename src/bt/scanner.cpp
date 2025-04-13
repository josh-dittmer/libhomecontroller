#include "homecontroller/bt/scanner.h"

namespace hc {
namespace bt {

bool Scanner::start(const std::set<std::string>& addresses) {
    m_logger.verbose("start(): Starting scan loop...");

    m_addresses = addresses;

    m_loop_thread = std::thread(&Scanner::loop_thread, this);

    return true;
}

void Scanner::shutdown_sync() {
    std::unique_lock<std::mutex> lock(m_mutex_adapter);

    if (m_adapter != nullptr) {
        gattlib_adapter_scan_disable(m_adapter);
        m_should_exit = true;
    }

    lock.unlock();

    for (auto& c : m_connections) {
        c.second->shutdown(); // locks conn mutex
    }

    m_logger.verbose("shutdown(): Waiting for loop thread to exit...");

    if (m_loop_thread.joinable()) {
        m_loop_thread.join();
    }
}

std::shared_ptr<Connection>
Scanner::get_connection(const std::string& address) {
    std::lock_guard<std::mutex> lock(m_mutex_adapter);

    if (m_scanning) {
        return nullptr;
    }

    auto mit = m_connections.find(address);
    if (mit == m_connections.end()) {
        return nullptr;
    }

    return mit->second;
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
    Scanner* instance = reinterpret_cast<Scanner*>(data);

    while (!instance->m_should_exit) {
        std::unique_lock<std::mutex> lock_adapter(instance->m_mutex_adapter);
        std::unique_lock<std::mutex> lock_connection(
            instance->m_shared_mutex_connection, std::defer_lock);

        if (gattlib_adapter_open(nullptr, &instance->m_adapter)) {
            instance->m_logger.error("Failed to open adapter!");
            return nullptr;
        }

        instance->m_scanning = true;

        lock_adapter.unlock();
        lock_connection.lock();

        instance->m_logger.verbose("scan_task(): Enabling scanning for:");
        for (auto& addr : instance->m_addresses) {
            instance->m_logger.verbose("scan_task(): [" + addr + "]");
        }

        if (gattlib_adapter_scan_enable(instance->m_adapter,
                                        Scanner::on_device_discovered, 0,
                                        instance) != GATTLIB_SUCCESS) {
            instance->m_logger.error("Failed to start scan! Retrying in 5s...");

            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            continue;
        }

        gattlib_adapter_scan_disable(instance->m_adapter);

        instance->m_logger.verbose("scan_task(): Finished scanning");

        lock_adapter.lock();
        instance->m_scanning = false;
        lock_adapter.unlock();

        // starts all connection threads
        lock_connection.unlock();

        for (auto& c : instance->m_connections) {
            c.second->await_finish();
        }

        lock_adapter.lock();
        if (instance->m_adapter != nullptr) {
            gattlib_adapter_close(instance->m_adapter);
        }
    }

    return nullptr;
}

void Scanner::on_device_discovered(gattlib_adapter_t* adapter,
                                   const char* addr_cstr, const char* name_cstr,
                                   void* data) {
    Scanner* instance = reinterpret_cast<Scanner*>(data);

    std::lock_guard<std::mutex> lock(instance->m_mutex_adapter);

    if (addr_cstr == nullptr) {
        return;
    }

    std::string address(addr_cstr);
    std::string name(name_cstr != nullptr ? name_cstr : "???");

    if (instance->m_addresses.find(address) != instance->m_addresses.end()) {
        instance->m_logger.verbose("on_device_discovered(): " + name + " @ [" +
                                   address + "] discovered");

        instance->create_connection(adapter, address, name);

        if (instance->m_connections.size() >= instance->m_addresses.size()) {
            gattlib_adapter_scan_disable(instance->m_adapter);
        }
    }
}

void Scanner::create_connection(gattlib_adapter_t* adapter,
                                const std::string& address,
                                const std::string& name) {
    std::shared_ptr<Connection> conn_ptr = std::make_shared<Connection>(
        std::ref(m_shared_mutex_connection), adapter, address, name);

    conn_ptr->start();

    m_connections.insert_or_assign(address, conn_ptr);
}

} // namespace bt
} // namespace hc