#include "SerialReader.h"
#include <iostream>
#include <sstream>

SerialReader::SerialReader(asio::io_context& io) : m_serial(io) {}

SerialReader::~SerialReader() {
    close();
}

bool SerialReader::open(const std::string& port_name, unsigned int baud_rate) {
    try {
        m_serial.open(port_name);
        m_serial.set_option(asio::serial_port_base::baud_rate(baud_rate));
        m_isOpen = true;
        std::cout << "Opened Serial Port: " << port_name << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to open serial port " << port_name << ": " << e.what() << std::endl;
        return false;
    }
}

void SerialReader::close() {
    if (m_isOpen) {
        asio::error_code ec;
        m_serial.close(ec);
        m_isOpen = false;
    }
}

void SerialReader::start(std::function<void(const std::string&, const std::string&)> onMessageCb) {
    if (!m_isOpen) return;
    m_onMessageCb = onMessageCb;
    doRead();
}

void SerialReader::doRead() {
    asio::async_read_until(m_serial, m_readBuf, '\n',
        [this](asio::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::istream is(&m_readBuf);
                std::string line;
                std::getline(is, line);
                
                // Parse 0x100,45
                size_t commaPos = line.find(',');
                if (commaPos != std::string::npos) {
                    std::string canId = line.substr(0, commaPos);
                    std::string data = line.substr(commaPos + 1);
                    // trim carriage return if present
                    if (!data.empty() && data.back() == '\r') {
                        data.pop_back();
                    }
                    if (m_onMessageCb) {
                        m_onMessageCb(canId, data);
                    }
                }
                doRead();
            } else {
                std::cerr << "Serial read error: " << ec.message() << std::endl;
                close();
            }
        });
}
