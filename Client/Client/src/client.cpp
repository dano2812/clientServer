#include "Client.h"

Client::Client(asio::io_context& io_context) : _io_context(io_context), _socket(io_context), _deadline(io_context), heartbeat_timer_(io_context) {}

void Client::Start(tcp::resolver::results_type endpoints) {
    _endpoints = endpoints;
    StartConnect(_endpoints.begin());

    // Start the deadline actor. You will note that we're not setting any
    // particular deadline here. Instead, the connect and input actors will
    // update the deadline prior to each asynchronous operation.
    _deadline.async_wait(std::bind(&Client::CheckDeadline, this));
}

void Client::Stop() {
    _stopped = true;
    std::error_code ignored_error;
    _socket.close(ignored_error);
    _deadline.cancel();
    heartbeat_timer_.cancel();
}

void Client::Write(const ChatMessage& msg) {
    asio::post(_io_context, [this, msg]() {
        bool write_in_progress = !_write_msgs.empty();
        _write_msgs.push_back(msg);
        if (!write_in_progress) {
            DoWrite();
        }
    });
}

bool Client::IsClosed() {
    return _stopped;
}

void Client::StartConnect(tcp::resolver::results_type::iterator endpoint_iter) {
    if (endpoint_iter != _endpoints.end()) {
        std::cout << "Trying " << endpoint_iter->endpoint() << "...\n";

        // Set a deadline for the connect operation.
        _deadline.expires_after(std::chrono::seconds(10));

        // Start the asynchronous connect operation.
        _socket.async_connect(endpoint_iter->endpoint(), std::bind(&Client::HandleConnect, this, _1, endpoint_iter));
    } else {
        // There are no more endpoints to try. Shut down the client.
        Stop();
    }
}

void Client::HandleConnect(const std::error_code& error, tcp::resolver::results_type::iterator endpoint_iter) {
    if (_stopped)
        return;

    if (!_socket.is_open()) {
        std::cout << "Connect timed out\n";

        // Try the next available endpoint.
        StartConnect(++endpoint_iter);
    }
    // Check if the connect operation failed before the deadline expired.
    else if (error) {
        std::cout << "Connect error: " << error.message() << "\n";

        _socket.close();

        // Try the next available endpoint.
        StartConnect(++endpoint_iter);
    }
    // Otherwise we have successfully established a connection.
    else {
        std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";

        // Read exisitng messages on the server
        DoReadHeader();
    }
}

void Client::DoReadHeader() {
    asio::async_read(_socket, asio::buffer(_read_msg.Data(), kHeaderLength), [this](std::error_code ec, std::size_t /*length*/) {
        if (!ec && _read_msg.DecodeHeader()) {
            DoReadBody();
        } else {
            _socket.close();
        }
    });
}

void Client::DoReadBody() {
    asio::async_read(_socket, asio::buffer(_read_msg.Body(), _read_msg.BodyLength()), [this](std::error_code ec, std::size_t /*length*/) {
        if (!ec) {
            std::cout.write(_read_msg.Body(), _read_msg.BodyLength());
            std::cout << "\n";
            DoReadHeader();
        } else {
            _socket.close();
        }
    });
}

void Client::DoWrite() {
    _deadline.expires_after(std::chrono::seconds(5));

    asio::async_write(
        _socket, asio::buffer(_write_msgs.front().Data(), _write_msgs.front().Length()), [this](std::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                SetInactiveTimeOut();
                _write_msgs.pop_front();
                if (!_write_msgs.empty()) {
                    DoWrite();
                }
            } else {
                std::cout << "Write error: " << ec.message() << "\n";
                ec.clear();
            }
        });
}

void Client::CheckDeadline() {
    if (_stopped)
        return;

    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (_deadline.expiry() <= steady_timer::clock_type::now()) {
        _socket.close();
        _stopped = true;

        std::cout << "Operation timed out! Session closed.\n";
        std::cout << "Press enter to exit . . .\n";
    }

    // Put the actor back to sleep.
    _deadline.async_wait(std::bind(&Client::CheckDeadline, this));
}

// Set inactive time.
// When time elapses, without clients action, session is automatically closed.
void Client::SetInactiveTimeOut() {
    _deadline.expires_at(steady_timer::clock_type::now() + std::chrono::seconds(10));
}