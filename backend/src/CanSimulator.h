#pragma once

#include <string>
#include <mutex>
#include <nlohmann/json.hpp> // Wait, we need json. Crow has its own JSON, or we can use nlohmann. Crow's JSON is crow::json. Let's use crow::json in main.cpp, but here we can just pass strings or use simple structs.

struct VehicleState {
    int steeringAngle = 0; // -180 to 180
    int brakePressure = 0; // 0 to 100
    int throttlePosition = 0; // 0 to 100
    int speed = 0; // 0 to 250 km/h
    int rpm = 800; // 0 to 8000
    std::string gear = "P"; // P, R, N, D
};

class CanSimulator {
public:
    CanSimulator();
    
    // Process a CAN message like "ID: 0x100, Data: 45"
    // Returns true if state changed
    bool processMessage(const std::string& canIdStr, const std::string& dataStr);
    
    VehicleState getState();

private:
    VehicleState m_state;
    std::mutex m_mutex;
};
