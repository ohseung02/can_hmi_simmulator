#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>

struct LogEntry {
    int time_ms;
    std::string canId;
    std::string data;
};

class LogReplayer {
public:
    LogReplayer();
    ~LogReplayer();

    bool loadLog(const std::string& filepath);
    void start(std::function<void(const std::string&, const std::string&)> onMessageCb);
    void stop();
    bool isRunning() const;

private:
    void replayThreadFunc();

    std::vector<LogEntry> m_entries;
    std::atomic<bool> m_running{false};
    std::thread m_thread;
    std::function<void(const std::string&, const std::string&)> m_onMessageCb;
};
