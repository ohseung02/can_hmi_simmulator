#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <asio.hpp>

class SerialReader {
public:
    SerialReader(asio::io_context& io);
    ~SerialReader();

    bool open(const std::string& port_name, unsigned int baud_rate = 115200);
    void close();
    void start(std::function<void(const std::string&, const std::string&)> onMessageCb);

private:
    void doRead();

    asio::serial_port m_serial;
    asio::streambuf m_readBuf;
    std::function<void(const std::string&, const std::string&)> m_onMessageCb;
    bool m_isOpen{false};
};
