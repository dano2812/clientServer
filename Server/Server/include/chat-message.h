#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

const int kMaxBodyLength = 512;
const int kHeaderLength = 4;

class ChatMessage {
public:
    ChatMessage() : _body_length(0) {}

    const char* Data() const {
        return _data;
    }

    char* Data() {
        return _data;
    }

    size_t Length() const {
        return kHeaderLength + _body_length;
    }

    const char* Body() const {
        return _data + kHeaderLength;
    }

    char* Body() {
        return _data + kHeaderLength;
    }

    size_t BodyLength() const {
        return _body_length;
    }

    void BodyLength(size_t length) {
        _body_length = length;
        if (_body_length > kMaxBodyLength)
            _body_length = kMaxBodyLength;
    }

    bool DecodeHeader() {
        using namespace std;  // For strncat and atoi.
        char header[kHeaderLength + 1] = "";
        strncat_s(header, _data, kHeaderLength);
        _body_length = atoi(header);
        if (_body_length > kMaxBodyLength) {
            _body_length = 0;
            return false;
        }
        return true;
    }

    void EncodeHeader() {
        using namespace std;  // For sprintf and memcpy.
        char header[kHeaderLength + 1] = "";
        sprintf_s(header, "%4d", int(_body_length));
        memcpy(_data, header, kHeaderLength);
    }

private:
    char _data[kHeaderLength + kMaxBodyLength];
    size_t _body_length;
};
