#pragma once

#include <string>
#include <mutex>
#include <unordered_map>

#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>

struct VehicleState {
    int steeringAngle = 0; // -180 to 180
    int brakePressure = 0; // 0 to 100
    int throttlePosition = 0; // 0 to 100
    int speed = 0; // 0 to 250 km/h
    int rpm = 800; // 0 to 8000
    std::string gear = "P"; // P, R, N, D
    int climateTemp = 24; // 18 to 30 C
};

struct CanMsg {
    int id;
    int data;
    // priority_queue is a max-heap. We want lowest ID to be at the top.
    // So if this.id > other.id, we return true (meaning this is "less" than other).
    bool operator<(const CanMsg& other) const {
        return id > other.id;
    }
};

class CanSimulator {
public:
    CanSimulator();
    ~CanSimulator();
    
    void start();
    void stop();

    // Process a CAN message like "ID: 0x100, Data: 45"
    // Returns true if state changed (Not used much directly now)
    bool processMessage(const std::string& canIdStr, const std::string& dataStr);
    
    VehicleState getState() const;
    std::unordered_map<std::string, int> popStats();
    
    // Returns {totalSize, {id -> count}}
    std::pair<size_t, std::unordered_map<int, int>> getQueueInfo() const;
    
    // Returns recently popped IDs and clears the buffer
    std::vector<int> popRecentPops();

private:
    void workerLoop();

    VehicleState m_state;
    std::unordered_map<std::string, int> m_stats;
    mutable std::mutex m_stateMutex;
    std::vector<int> m_recentPops;

    std::priority_queue<CanMsg> m_queue;
    std::unordered_map<int, int> m_queueCounts;
    mutable std::mutex m_queueMutex;
    std::condition_variable m_cv;
    std::thread m_worker;
    std::atomic<bool> m_running{false};
};
