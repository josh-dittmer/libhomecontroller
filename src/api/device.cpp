#include "homecontroller/api/device.h"

#include "homecontroller/api/device_data/rgb_lights.h"

#include <thread>

namespace hc {
namespace api {
template <typename StateType>
void Device<StateType>::start(const StartParams& start_params) {
    m_state = start_params.m_initial_state;

    register_events();

    m_client.start(
        start_params.m_gateway.m_url, start_params.m_gateway.m_namespace,
        create_handshake_msg(start_params.m_device_id, start_params.m_secret),
        start_params.m_reconn_delay, start_params.m_reconn_attempts);
}

template <typename StateType>
void Device<StateType>::await_finish_and_cleanup() {
    m_client.await_finish_and_cleanup();
}

template <typename StateType> void Device<StateType>::stop() {
    m_logger.verbose("Initiating client connection close...");

    m_client.shutdown();
}

template <typename StateType>
void Device<StateType>::update_state(StateType new_state) {
    m_state = new_state;

    ::sio::message::ptr state_update_msg = ::sio::object_message::create();
    state_update_msg->get_map()["data"] = serialize_state();

    // m_logger.verbose("update_state(): Sending update message to server");

    m_client.send("deviceStateChanged", state_update_msg);
}

template <typename StateType> void Device<StateType>::register_events() {
    sio::Client::EventCallback cb1 = [this](::sio::message::ptr msg) {
        std::string socket_id = msg->get_map()["socketId"]->get_string();

        ::sio::message::ptr reply_msg = ::sio::object_message::create();
        reply_msg->get_map()["socketId"] =
            ::sio::string_message::create(socket_id);
        reply_msg->get_map()["data"] = serialize_state();

        m_client.send("deviceCheckStateReply", reply_msg);
    };

    m_client.register_event("deviceCheckStateRequest", cb1);

    sio::Client::EventCallback cb2 = [this](::sio::message::ptr msg) {
        std::map<std::string, ::sio::message::ptr> data =
            msg->get_map()["data"]->get_map();

        on_command_received(data);
    };

    m_client.register_event("deviceCommand", cb2);

    sio::Client::EventCallback cb3 = [this](::sio::message::ptr) {
        m_logger.log("Device has been deleted! Shutting down...");
        stop();
    };

    m_client.register_event("deviceDeleted", cb3);
}

template <typename StateType>
::sio::message::ptr
Device<StateType>::create_handshake_msg(const std::string& device_id,
                                        const std::string& secret) {
    ::sio::message::ptr handshake_msg = ::sio::object_message::create();
    handshake_msg->get_map()["type"] = ::sio::string_message::create("device");
    handshake_msg->get_map()["deviceId"] =
        ::sio::string_message::create(device_id);
    handshake_msg->get_map()["key"] = ::sio::string_message::create(secret);

    return handshake_msg;
}

template class Device<hc::api::rgb_lights::State>;

} // namespace api
} // namespace hc