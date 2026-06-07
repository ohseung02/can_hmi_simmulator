#include "LogReplayer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>

LogReplayer::LogReplayer() {}

LogReplayer::~LogReplayer() {
    stop();
}

bool LogReplayer::loadLog(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    m_entries.clear();
    std::string line;
    // Skip header
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string timeStr, idStr, dataStr;
        
        if (std::getline(ss, timeStr, ',') &&
            std::getline(ss, idStr, ',') &&
            std::getline(ss, dataStr)) {
            
            try {
                LogEntry entry;
                entry.time_ms = std::stoi(timeStr);
                entry.canId = idStr;
                entry.data = dataStr;
                m_entries.push_back(entry);
            } catch (...) {
                // skip invalid lines
            }
        }
    }
    return !m_entries.empty();
}

void LogReplayer::start(std::function<void(const std::string&, const std::string&)> onMessageCb) {
    if (m_running) return;
    if (m_entries.empty()) return;

    m_onMessageCb = onMessageCb;
    m_running = true;
    m_thread = std::thread(&LogReplayer::replayThreadFunc, this);
}

void LogReplayer::stop() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool LogReplayer::isRunning() const {
    return m_running;
}

void LogReplayer::replayThreadFunc() {
    auto startTime = std::chrono::steady_clock::now();
    size_t currentIndex = 0;

    while (m_running && currentIndex < m_entries.size()) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

        if (elapsed_ms >= m_entries[currentIndex].time_ms) {
            if (m_onMessageCb) {
                m_onMessageCb(m_entries[currentIndex].canId, m_entries[currentIndex].data);
            }
            currentIndex++;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    m_running = false;
}
