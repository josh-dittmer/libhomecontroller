#pragma once

#include "homecontroller/socket.io/client.h"

#include <atomic>

namespace hc {
namespace api {
template <typename StateType> class Device {
  public:
    struct Gateway {
        std::string m_url;
        std::string m_namespace;
    };

    struct StartParams {
        Gateway m_gateway;

        std::string m_device_id;
        std::string m_secret;

        StateType m_initial_state;

        int m_reconn_delay;
        int m_reconn_attempts;
    };

    Device(const std::string& log_context) : m_logger(log_context) {}
    ~Device() {}

    void start(const StartParams& start_params);
    void await_finish_and_cleanup();

    void stop();

    bool is_client_running() { return m_client.is_running(); }

    const StateType get_state() const { return m_state; }

    const util::Logger& get_logger() const { return m_logger; }

  protected:
    virtual void
    on_command_received(std::map<std::string, ::sio::message::ptr>& data) = 0;

    virtual ::sio::message::ptr serialize_state() const = 0;

    void update_state(StateType new_state);

  private:
    void register_events();
    ::sio::message::ptr create_handshake_msg(const std::string& device_id,
                                             const std::string& secret);

    util::Logger m_logger;

    sio::Client m_client;

    std::atomic<StateType> m_state;
};
} // namespace api
} // namespace hc