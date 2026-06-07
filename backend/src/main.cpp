#include "crow.h"
#include "CanSimulator.h"
#include "SerialReader.h"
#include "LogReplayer.h"
#include <mutex>
#include <unordered_set>
#include <iostream>
#include <thread>
#include <asio.hpp>

int main() {
    crow::SimpleApp app;
    CanSimulator canSim;
    LogReplayer logReplayer;
    
    asio::io_context io;
    SerialReader serialReader(io);
    
    // Attempt to open serial port (change COM3 to your actual port if testing later)
    serialReader.open("COM3", 115200);

    std::recursive_mutex mtx;
    std::unordered_set<crow::websocket::connection*> users;

    auto broadcastState = [&]() {
        VehicleState state = canSim.getState();
        crow::json::wvalue res;
        res["type"] = "STATE";
        res["steering"] = state.steeringAngle;
        res["brake"] = state.brakePressure;
        res["throttle"] = state.throttlePosition;
        res["speed"] = state.speed;
        res["rpm"] = state.rpm;
        res["gear"] = state.gear;
        res["climateTemp"] = state.climateTemp;
        
        std::string broadcast_str = res.dump();
        
        std::lock_guard<std::recursive_mutex> _(mtx);
        for (auto u : users) {
            u->send_text(broadcast_str);
        }
    };

    auto onCanMessage = [&](const std::string& canId, const std::string& data) {
        if (canSim.processMessage(canId, data)) {
            broadcastState();
        }
    };

    // Start Serial reading
    serialReader.start(onCanMessage);
    
    // Start ASIO thread for serial
    std::thread asioThread([&io]() {
        io.run();
    });

    CROW_WEBSOCKET_ROUTE(app, "/ws")
      .onopen([&](crow::websocket::connection& conn) {
          CROW_LOG_INFO << "new websocket connection";
          std::lock_guard<std::recursive_mutex> _(mtx);
          users.insert(&conn);
          broadcastState(); // send initial state
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
          CROW_LOG_INFO << "websocket connection closed: " << reason;
          std::lock_guard<std::recursive_mutex> _(mtx);
          users.erase(&conn);
      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
          auto j = crow::json::load(data);
          if (!j) return;
          
          if (j.has("action")) {
              std::string action = j["action"].s();
              if (action == "replay_start") {
                  if (!logReplayer.isRunning()) {
                      logReplayer.loadLog("backend/drive_log.csv"); // relative path
                      logReplayer.start(onCanMessage);
                      std::cout << "Replay started." << std::endl;
                  }
              } else if (action == "replay_stop") {
                  logReplayer.stop();
                  std::cout << "Replay stopped." << std::endl;
              }
          } else if (j.has("canId") && j.has("data")) {
              std::string canId = j["canId"].s();
              std::string canData = j["data"].s();
              onCanMessage(canId, canData);
          }
      });

    std::cout << "Starting Server on port 18080..." << std::endl;
    app.port(18080).multithreaded().run();

    // Cleanup
    serialReader.close();
    io.stop();
    if (asioThread.joinable()) asioThread.join();
}
