#pragma once

#include "asio/buffer.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/read_until.hpp"
#include "asio/steady_timer.hpp"
#include "asio/write.hpp"
#include "asio/read.hpp"

#include <functional>
#include <iostream>
#include <string>
#include <deque>
#include <thread>

#include "chat-message.h"

typedef std::deque<ChatMessage> chat_message_queue;

using asio::steady_timer;
using asio::ip::tcp;
using std::placeholders::_1;

class Client {
public:
    Client(asio::io_context& io_context);

    void Start(tcp::resolver::results_type endpoints);

    // This function terminates all the actors to shut down the connection. It
    // may be called by the user of the client class, or by the class itself in
    // response to graceful termination or an unrecoverable error.
    void Stop();

    void Write(const ChatMessage& msg);

    bool IsClosed();

private:
    void StartConnect(tcp::resolver::results_type::iterator endpoint_iter);

    void HandleConnect(const std::error_code& error, tcp::resolver::results_type::iterator endpoint_iter);

    void DoReadHeader();

    void DoReadBody();

    void DoWrite();

    void CheckDeadline();

    // Set inactive time.
    // When time elapses without clients action, session is automatically closed.
    void SetInactiveTimeOut();

private:
    bool _stopped = false;
    tcp::resolver::results_type _endpoints;
    asio::io_context& _io_context;
    tcp::socket _socket;
    std::string input_buffer_;
    steady_timer _deadline;
    steady_timer heartbeat_timer_;

    ChatMessage _read_msg;
    chat_message_queue _write_msgs;
};
