#include "homecontroller/bt/device.h"

namespace hc {
namespace bt {

bool Device::connect() {
    // m_logger.log("Connecting to [" + m_address + "]...");

    std::unique_lock<std::mutex> lock(m_mutex);

    int ret = gattlib_connect(m_adapter, m_address.c_str(),
                              GATTLIB_CONNECTION_OPTIONS_NONE,
                              Device::on_device_connect, this);

    if (ret != GATTLIB_SUCCESS) {
        m_logger.debug("gattlib_connect() failed with code: " +
                       std::to_string(ret));
        return false;
    }

    m_logger.verbose("connect(): Waiting for connection to finish...");

    m_cv.wait(lock);

    m_logger.verbose("connect(): Connection finished!");

    return true;
}

void Device::on_device_connect(gattlib_adapter_t* adapter, const char* dst_cstr,
                               gattlib_connection_t* connection, int error,
                               void* data) {
    Device* instance = reinterpret_cast<Device*>(data);

    instance->m_logger.verbose("connect(): Callback invoked");

    if (error != GATTLIB_SUCCESS) {
        return;
    }

    instance->m_logger.verbose("connect(): .");
    std::unique_lock<std::mutex> lock(instance->m_mutex);
    instance->m_logger.verbose("connect(): ..");

    gattlib_primary_service_t* services;
    int num_services;

    gattlib_characteristic_t* characteristics;
    int num_characteristics;

    instance->m_cv.notify_all();
    // instance->m_logger.log("DEVICE CONNECTED!!! " + std::to_string(error));
}

} // namespace bt
} // namespace hc