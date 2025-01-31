#include "homecontroller/socket.io/client.h"

#include <chrono>
#include <thread>

namespace hc {
namespace sio {
void Client::register_event(const std::string& name, EventCallback callback) {
    m_events.insert(std::make_pair(name, callback));
}

void Client::start(const std::string& url, const std::string& nsp,
                   ::sio::message::ptr handshake_msg, int reconn_delay,
                   int reconn_attempts) {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_logger.verbose("Client::start(): Attempting connection to " + url +
                     "...");

    m_client = std::make_unique<::sio::client>();
    m_namespace = nsp;

    init_client(reconn_delay, reconn_attempts);

    m_connecting = true;
    m_client->connect(url, handshake_msg);

    m_logger.verbose(
        "Client::start(): Waiting for connection attempt to finish...");

    m_cv.wait(lock);

    m_logger.verbose("Client::start(): Connection attempt finished!");
}

void Client::send(const std::string& event_name, ::sio::message::ptr msg) {
    std::unique_lock<std::mutex> lock(m_mutex);

    if (!m_client || !m_client->opened()) {
        m_logger.warn("Client::send(): Cannot send, client is not opened!");
        return;
    }

    m_current_socket->emit(event_name, msg);
}

void Client::shutdown() {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_logger.verbose("Client::shutdown(): Sending signal...");

    m_cv.notify_all();
}

void Client::await_finish_and_cleanup() {
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_running) {
        m_logger.verbose(
            "Client::await_finish_and_cleanup(): Client is running, "
            "waiting for signal...");

        m_cv.wait(lock);

        m_logger.verbose("Client::await_finish_and_cleanup(): Received signal, "
                         "cleaning up...");

        m_current_socket->off_all();
        m_current_socket->off_error();
    }

    m_client->clear_con_listeners();

    if (m_connecting || (!m_client->opened() && m_running)) {
        m_logger.warn("Client is in the connecting state. The socket.io "
                      "client library will likely show an error, but this "
                      "can be safely ignored.");
    }

    // calls ::sio::client::sync_close() in destructor
    m_client.reset();

    m_running = false;
}

/*bool Client::is_opened() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_client && m_client->opened();
}*/

bool Client::is_running() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_running;
}

void Client::init_client(int reconn_delay, int reconn_attempts) {
    m_client->set_open_listener(std::bind(&Client::on_open, this));
    m_client->set_close_listener(
        std::bind(&Client::on_close, this, std::placeholders::_1));
    m_client->set_fail_listener(std::bind(&Client::on_fail, this));
    m_client->set_reconnecting_listener(
        std::bind(&Client::on_reconnecting, this));

    m_client->set_reconnect_delay(reconn_delay);
    m_client->set_reconnect_delay_max(reconn_delay);
    m_client->set_reconnect_attempts(reconn_attempts);

    m_client->set_logs_quiet();
}

void Client::on_open() {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_current_socket = m_client->socket(m_namespace);

    m_current_socket->on_error([&, this](const ::sio::message::ptr& msg) {
        m_logger.verbose("Socket error handler invoked");
        m_logger.error("Failed to connect to server!");
        shutdown();
    });

    for (auto& x : m_events) {
        ::sio::socket::event_listener cb = [&, this](::sio::event& event) {
            m_logger.verbose("Received event: [" + event.get_name() + "]");

            try {
                x.second(event.get_message());
            } catch (std::exception& e) {
                m_logger.error("Event handler failed: " +
                               std::string(e.what()));
            }
        };

        m_logger.verbose("Registered event [" + x.first + "]");

        m_current_socket->on(x.first, cb);
    }

    if (!m_running) {
        m_cv.notify_all();
        m_running = true;
    }

    m_connecting = false;

    m_logger.log("Connected opened successfully");
}

void Client::on_close(const ::sio::client::close_reason& reason) {
    m_logger.debug(
        "Message placed here to verify proper cleanup, you should "
        "not be seeing this unless client was disconnected by the server.");
}

void Client::on_fail() {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_logger.verbose("Client::on_fail(): Initial connection failed");
    m_logger.error("Failed to connect to server!");

    m_running = false;
    m_connecting = false;
    m_cv.notify_all();
}

void Client::on_reconnecting() {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_logger.warn("Connection lost! Reconnecting...");

    m_connecting = true;
}

} // namespace sio
} // namespace hc