#pragma once

#define SIO_TLS 1

#include "homecontroller/util/logger.h"

#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <sio_client.h>

namespace hc {
namespace sio {
class Client {
  public:
    Client() : m_logger("SIOClient"), m_running(false), m_connecting(false) {}
    ~Client() {}

    typedef std::function<void(::sio::message::ptr)> EventCallback;

    void register_event(const std::string& name, EventCallback callback);

    void start(const std::string& url, const std::string& nsp,
               ::sio::message::ptr handshake_msg, int reconn_delay,
               int reconn_attempts);

    void send(const std::string& event_name, ::sio::message::ptr msg);

    void shutdown();

    void await_finish_and_cleanup();

    // bool is_opened() const;
    bool is_running();

  private:
    void init_client(int reconn_delay, int reconn_attempts);

    void on_open();
    void on_close(const ::sio::client::close_reason& reason);
    void on_fail();
    void on_reconnecting();

    util::Logger m_logger;

    std::string m_namespace;

    std::map<std::string, EventCallback> m_events;

    std::unique_ptr<::sio::client> m_client;
    ::sio::socket::ptr m_current_socket;

    std::mutex m_mutex;
    std::condition_variable m_cv;

    bool m_running;
    bool m_connecting;
};
} // namespace sio
} // namespace hc