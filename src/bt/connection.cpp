#include "homecontroller/bt/connection.h"

#include "homecontroller/util/string.h"

#include <thread>

namespace hc {
namespace bt {

void Connection::start() {
    m_loop_thread = std::thread(&Connection::loop_thread, this);

    m_logger.verbose("start(): [" + m_address + "] started");
}

void Connection::shutdown() {
    std::unique_lock<std::mutex> lock(m_mutex_ref);

    if (!m_should_exit && m_running) {
        m_should_exit = true;
        m_cv_queue.notify_all();

        m_logger.verbose("shutdown(): [" + m_address + "] started");
    }
}

void Connection::await_finish() {
    m_logger.verbose("await_finish(): [" + m_address + "] awaiting exit");

    if (m_loop_thread.joinable()) {
        m_loop_thread.join();
    }

    m_logger.verbose("await_finish(): [" + m_address + "] exited");
}

void Connection::write_char(int test) {
    std::lock_guard<std::mutex> lock(m_mutex_ref);

    if (!m_running) {
        return;
    }

    m_queue.push(test);
    m_cv_queue.notify_all();
}

void Connection::loop_thread() {
    std::unique_lock<std::mutex> lock(m_mutex_ref);

    m_logger.verbose("loop_thread(): [" + m_address + "] started");

    m_running = true;

    int ret = gattlib_connect(m_adapter, m_address.c_str(),
                              GATTLIB_CONNECTION_OPTIONS_NONE,
                              Connection::on_device_connect, this);

    if (ret != GATTLIB_SUCCESS) {
        m_logger.verbose("loop_thread(): gattlib_connect() failed with code: " +
                         std::to_string(ret));
        m_logger.error("[" + m_address + "]: connection failed");

        return;
    }

    m_logger.verbose("loop_thread(): [" + m_address + "] awaiting finish");

    m_cv_finished.wait(lock, [this] { return !m_running; });

    m_logger.verbose("loop_thread(): [" + m_address + "] exited");
}

void Connection::on_device_connect(gattlib_adapter_t* adapter,
                                   const char* dst_cstr,
                                   gattlib_connection_t* connection, int error,
                                   void* data) {
    Connection* instance = reinterpret_cast<Connection*>(data);

    if (error != GATTLIB_SUCCESS || dst_cstr == nullptr) {
        instance->m_logger.verbose("on_device_connect(): Failed with code: " +
                                   std::to_string(error));
        return;
    }

    std::unique_lock<std::mutex> lock(instance->m_mutex_ref);

    gattlib_register_on_disconnect(connection, Connection::on_device_disconnect,
                                   data);

    std::string address(dst_cstr);

    instance->m_logger.log(instance->m_name + " @ [" + address + "] connected");

    instance->perform_discovery(connection);

    while (!instance->m_should_exit) {
        instance->m_cv_queue.wait(lock, [&] {
            return !instance->m_queue.empty() || instance->m_should_exit;
        });

        while (!instance->m_queue.empty()) {
            std::cout << instance->m_queue.front() << std::endl;
            /*instance->m_logger.verbose(
                "on_device_connect(): Queue item: " +
                std::to_string(instance->m_queue.front()));*/
            instance->m_queue.pop();
        }
    }

    instance->m_logger.verbose("on_device_connect(): [" + instance->m_address +
                               "] disconnecting...");

    instance->m_running = false;
    gattlib_disconnect(connection, true);

    instance->m_cv_finished.notify_all();

    instance->m_logger.log(instance->m_name + " @ [" + address +
                           "] disconnected");
    instance->m_logger.verbose("on_device_connect(): [" + instance->m_address +
                               "] exited");
}

void Connection::on_device_disconnect(gattlib_connection_t* connection,
                                      void* data) {
    Connection* instance = reinterpret_cast<Connection*>(data);

    if (instance->m_running) {
        instance->shutdown();
    }

    instance->m_logger.verbose("on_device_disconnect(): [" +
                               instance->m_address + "] disconnected");
}

void Connection::perform_discovery(gattlib_connection_t* connection) {
    gattlib_primary_service_t* services;
    int num_services;

    if (gattlib_discover_primary(connection, &services, &num_services) !=
        GATTLIB_SUCCESS) {
        m_logger.verbose("Failed to discover primary services for [" +
                         m_address + "]");
        return;
    }

    m_logger.verbose("*** Primary services for [" + m_address + "] ***");
    for (int i = 0; i < num_services; i++) {
        char uuid_cstr[MAX_LEN_UUID_STR + 1];

        if (gattlib_uuid_to_string(&services[i].uuid, uuid_cstr,
                                   sizeof(uuid_cstr)) != GATTLIB_SUCCESS) {
            continue;
        }

        std::string uuid(uuid_cstr);
        std::string start_handle =
            hc::util::str::to_hex(services[i].attr_handle_start, 2);
        std::string end_handle =
            hc::util::str::to_hex(services[i].attr_handle_end, 2);

        m_logger.verbose(std::to_string(i) + ": service [" + uuid +
                         "]: start handle [" + start_handle +
                         "], end handle [" + end_handle + "]");
    }
    free(services);

    gattlib_characteristic_t* characteristics;
    int num_characteristics;

    if (gattlib_discover_char(connection, &characteristics,
                              &num_characteristics) != GATTLIB_SUCCESS) {
        m_logger.verbose("Failed to discover characteristics for [" +
                         m_address + "]");
        return;
    }

    m_logger.verbose("*** Characteristics for [" + m_address + "] ***");
    for (int i = 0; i < num_characteristics; i++) {
        char uuid_cstr[MAX_LEN_UUID_STR + 1];

        if (gattlib_uuid_to_string(&services[i].uuid, uuid_cstr,
                                   sizeof(uuid_cstr)) != GATTLIB_SUCCESS) {
            continue;
        }

        std::string uuid(uuid_cstr);
        std::string properties =
            hc::util::str::to_hex(characteristics[i].properties, 2);
        std::string value_handle =
            hc::util::str::to_hex(characteristics[i].value_handle, 2);

        m_logger.verbose(std::to_string(i) + ": characteristic [" + uuid +
                         "]: properties [" + properties + "] tvalue handle [" +
                         value_handle + "]");
    }
    free(characteristics);
}

} // namespace bt
} // namespace hc