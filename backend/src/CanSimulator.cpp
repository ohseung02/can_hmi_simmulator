#include "CanSimulator.h"
#include <iostream>
#include <sstream>

CanSimulator::CanSimulator() {
}

bool CanSimulator::processMessage(const std::string& canIdStr, const std::string& dataStr) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Parse ID (expecting format like 0x100)
    int id = 0;
    try {
        id = std::stoi(canIdStr, nullptr, 16);
    } catch (...) {
        return false; // Invalid ID format
    }

    // Parse Data
    int data = 0;
    try {
        data = std::stoi(dataStr);
    } catch (...) {
        return false; // Invalid Data format
    }

    bool changed = false;

    switch (id) {
        case 0x100: // Steering
            m_state.steeringAngle = data;
            if (m_state.steeringAngle > 180) m_state.steeringAngle = 180;
            if (m_state.steeringAngle < -180) m_state.steeringAngle = -180;
            changed = true;
            break;
        case 0x101: // Brake
            m_state.brakePressure = data;
            if (m_state.brakePressure > 100) m_state.brakePressure = 100;
            if (m_state.brakePressure < 0) m_state.brakePressure = 0;
            changed = true;
            break;
        case 0x102: // Throttle
            m_state.throttlePosition = data;
            if (m_state.throttlePosition > 100) m_state.throttlePosition = 100;
            if (m_state.throttlePosition < 0) m_state.throttlePosition = 0;
            changed = true;
            break;
        case 0x200: // Speed
            m_state.speed = data;
            if (m_state.speed > 250) m_state.speed = 250;
            if (m_state.speed < 0) m_state.speed = 0;
            
            // simple rpm simulation
            m_state.rpm = 800 + (m_state.speed * 25);
            changed = true;
            break;
        case 0x300: // Gear (Data: 0=P, 1=R, 2=N, 3=D)
            if (data == 0) m_state.gear = "P";
            else if (data == 1) m_state.gear = "R";
            else if (data == 2) m_state.gear = "N";
            else if (data == 3) m_state.gear = "D";
            changed = true;
            break;
        case 0x400: // Climate Temp
            m_state.climateTemp = data;
            changed = true;
            break;
        default:
            break;
    }

    return changed;
}

VehicleState CanSimulator::getState() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state;
}
