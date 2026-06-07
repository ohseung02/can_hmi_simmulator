# CAN-based Vehicle Infotainment (HMI) Simulator

## Project Overview
This project is a full-stack web simulator that mimics a vehicle infotainment (HMI) system operating based on Controller Area Network (CAN) bus data. A high-performance backend server built in C++ receives and parses CAN signals, broadcasting them in real-time to a web frontend via WebSockets. It also supports Hardware-In-the-Loop (HIL) integration using actual Arduino-based hardware.

## Tech Stack
- Backend (Server): C++17, CMake, Crow (WebSocket & HTTP Framework), Asio (Serial Communication)
- Frontend (Web UI): Vanilla HTML5, CSS3, JavaScript (ES6+), WebSocket API
- Hardware (Optional): Arduino UNO, MCP2515 CAN Shield, DHT11 (Temperature Sensor)
- Version Control: Git

## Key Features

### 1. Real-time HMI Visualization
- Controls: Intuitive graphics and gauge bars for steering wheel angle (-180 to 180 degrees), brake pressure, and throttle position.
- Cluster: Real-time rendering of vehicle speed, RPM, gear status (P/R/N/D), and cabin temperature.
- Design Principle: Premium dark mode UI based on Glassmorphism, tailored to modern vehicle design trends.

### 2. High-performance C++ WebSocket Server
- Custom asynchronous WebSocket server built on the Crow framework to broadcast hundreds of CAN messages per second to the frontend without delay.

### 3. Hardware-In-the-Loop (HIL) Simulation Support
- Arduino CAN Shield Integration: Hardware signals captured from an actual CAN bus are read by the Arduino and transmitted to the C++ server via a USB serial port (COM).
- The Asio library is used to dedicate a separate thread within the C++ server for real-time parsing of serial data.

### 4. CAN Log Replay
- Implemented a replay engine (LogReplayer) that automatically plays back recorded driving logs (CSV format) in chronological order, allowing testing even without physical hardware.

## Development Process & Troubleshooting
- Requirements & Design: Transitioned from a one-way UI to a real-time bi-directional communication architecture between the C++ backend and the web.
- Phase 1 (Software Simulation): Developed C++ backend logic and web HMI. Enabled immediate UI response via manual CAN ID inputs.
- Phase 2 (Hardware Integration): Analyzed actual Arduino TX/RX code to add serial communication modules and the driving log replay feature.
- Troubleshooting (Deadlock Resolution): Resolved thread deadlocks occurring during WebSocket connections and CAN message broadcasting by introducing std::recursive_mutex, ensuring server stability.

## How to Run
1. Build: cmake -B build and cmake --build build --config Release
2. Run Server: ./build/CanServer.exe
3. Open UI: Open frontend/index.html in your browser
