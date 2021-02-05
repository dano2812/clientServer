#pragma once

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <asio.hpp>
#include <chat_message.h>

using asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<ChatMessage> chat_message_queue;

//----------------------------------------------------------------------

class ChatParticipant {
public:
    virtual ~ChatParticipant() {}
    virtual void Deliver(const ChatMessage& msg) = 0;
};

typedef std::shared_ptr<ChatParticipant> chat_participant_ptr;

//----------------------------------------------------------------------

class ChatRoom {
public:
    void Join(chat_participant_ptr participant) {
        _participants.insert(participant);
        for (auto msg : _recent_msgs)
            participant->Deliver(msg);
    }

    void Leave(chat_participant_ptr participant) {
        _participants.erase(participant);
    }

    void Deliver(const ChatMessage& msg) {
        _recent_msgs.push_back(msg);
        while (_recent_msgs.size() > max_recent_msgs)
            _recent_msgs.pop_front();

        for (auto participant : _participants)
            participant->Deliver(msg);
    }

private:
    std::set<chat_participant_ptr> _participants;
    enum { max_recent_msgs = 100 };
    chat_message_queue _recent_msgs;
};

//----------------------------------------------------------------------

class ChatSession : public ChatParticipant, public std::enable_shared_from_this<ChatSession> {
public:
    ChatSession(tcp::socket socket, ChatRoom& room) : _socket(std::move(socket)), _room(room) {}

    void Start() {
        _room.Join(shared_from_this());
        DoReadHeader();
    }

    void Deliver(const ChatMessage& msg) {
        bool write_in_progress = !_write_msgs.empty();
        _write_msgs.push_back(msg);
        if (!write_in_progress) {
            DoWrite();
        }
    }

private:
    void DoReadHeader() {
        auto self(shared_from_this());
        asio::async_read(_socket, asio::buffer(_read_msg.Data(), kHeaderLength), [this, self](std::error_code ec, std::size_t) {
            if (!ec && _read_msg.DecodeHeader()) {
                DoReadBody();
            } else {
                _room.Leave(shared_from_this());
            }
        });
    }

    void DoReadBody() {
        auto self(shared_from_this());
        asio::async_read(_socket, asio::buffer(_read_msg.Body(), _read_msg.BodyLength()), [this, self](std::error_code ec, std::size_t) {
            if (!ec) {
                _room.Deliver(_read_msg);
                DoReadHeader();
            } else {
                _room.Leave(shared_from_this());
            }
        });
    }

    void DoWrite() {
        auto self(shared_from_this());
        asio::async_write(
            _socket, asio::buffer(_write_msgs.front().Data(), _write_msgs.front().Length()), [this, self](std::error_code ec, std::size_t) {
                if (!ec) {
                    _write_msgs.pop_front();
                    if (!_write_msgs.empty()) {
                        DoWrite();
                    }
                } else {
                    _room.Leave(shared_from_this());
                }
            });
    }

    tcp::socket _socket;
    ChatRoom& _room;
    ChatMessage _read_msg;
    chat_message_queue _write_msgs;
};

//----------------------------------------------------------------------

class ChatServer {
public:
    ChatServer(asio::io_context& io_context, const tcp::endpoint& endpoint) : _acceptor(io_context, endpoint) {
        DoAccept();
    }

private:
    void DoAccept() {
        _acceptor.async_accept([this](std::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<ChatSession>(std::move(socket), _room)->Start();
            }

            DoAccept();
        });
    }

    tcp::acceptor _acceptor;
    ChatRoom _room;
};

//----------------------------------------------------------------------

//*/