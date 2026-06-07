#include "CanSimulator.h"
#include <iostream>
#include <sstream>

CanSimulator::CanSimulator() {
}

CanSimulator::~CanSimulator() {
    stop();
}

void CanSimulator::start() {
    if (!m_running) {
        m_running = true;
        m_worker = std::thread(&CanSimulator::workerLoop, this);
    }
}

void CanSimulator::stop() {
    if (m_running) {
        m_running = false;
        m_cv.notify_all();
        if (m_worker.joinable()) {
            m_worker.join();
        }
    }
}

void CanSimulator::workerLoop() {
    while (m_running) {
        CanMsg msg;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_cv.wait(lock, [this] { return !m_queue.empty() || !m_running; });
            
            if (!m_running) break;
            if (m_queue.empty()) continue;
            
            msg = m_queue.top();
            m_queue.pop();
        }

        // Simulate CAN bus bandwidth limit (~500 msgs/sec max) by taking 2ms to "transmit" each message
        std::this_thread::sleep_for(std::chrono::milliseconds(2));

        // Process the message (State update)
        std::lock_guard<std::mutex> stateLock(m_stateMutex);
        
        // Record stats (format ID as 0x...)
        char hexId[10];
        snprintf(hexId, sizeof(hexId), "0x%X", msg.id);
        m_stats[std::string(hexId)]++;

        switch (msg.id) {
            case 0x100: // Steering
                m_state.steeringAngle = msg.data;
                break;
            case 0x101: // Brake
                m_state.brakePressure = msg.data;
                break;
            case 0x102: // Throttle
                m_state.throttlePosition = msg.data;
                break;
            case 0x200: // Speed
                m_state.speed = msg.data;
                break;
            case 0x300: // Gear
                if (msg.data == 0) m_state.gear = "P";
                else if (msg.data == 1) m_state.gear = "R";
                else if (msg.data == 2) m_state.gear = "N";
                else if (msg.data == 3) m_state.gear = "D";
                break;
            case 0x400: // Climate
                m_state.climateTemp = msg.data;
                break;
            default:
                break;
        }
    }
}

bool CanSimulator::processMessage(const std::string& canIdStr, const std::string& dataStr) {
    int id = 0;
    try {
        id = std::stoi(canIdStr, nullptr, 16);
    } catch (...) {
        return false;
    }

    int data = 0;
    try {
        data = std::stoi(dataStr);
    } catch (...) {
        return false;
    }

    // Push to Priority Queue instead of processing immediately
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        // Prevent queue from consuming infinite memory if flooded indefinitely
        if (m_queue.size() < 10000) {
            m_queue.push({id, data});
            m_cv.notify_one();
        }
    }
    
    return true;
}


std::unordered_map<std::string, int> CanSimulator::popStats() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    std::unordered_map<std::string, int> current_stats = m_stats;
    m_stats.clear();
    return current_stats;
}

VehicleState CanSimulator::getState() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_state;
}
