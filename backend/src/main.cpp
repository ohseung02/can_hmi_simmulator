#include "crow.h"
#include "CanSimulator.h"
#include <mutex>
#include <unordered_set>
#include <iostream>

int main() {
    crow::App<crow::CORSHandler> app;
    CanSimulator canSim;

    std::mutex mtx;
    std::unordered_set<crow::websocket::connection*> users;

    // Route to serve frontend (if we were to serve it from C++, but for now we might just open html file directly or serve via python.
    // Let's actually serve the frontend directory!
    // Since we don't know exactly where we run from, we will assume we run from the build folder which is alongside frontend.
    // Or we just serve a static folder if requested. We won't do static folder, the user can just open the HTML file locally,
    // but Crow WebSockets requires connection. So opening HTML directly in browser works fine (file://...).

    CROW_WEBSOCKET_ROUTE(app, "/ws")
      .onopen([&](crow::websocket::connection& conn) {
          CROW_LOG_INFO << "new websocket connection";
          std::lock_guard<std::mutex> _(mtx);
          users.insert(&conn);
          
          // Send initial state
          VehicleState state = canSim.getState();
          crow::json::wvalue res;
          res["type"] = "STATE";
          res["steering"] = state.steeringAngle;
          res["brake"] = state.brakePressure;
          res["throttle"] = state.throttlePosition;
          res["speed"] = state.speed;
          res["rpm"] = state.rpm;
          res["gear"] = state.gear;
          conn.send_text(res.dump());
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
          CROW_LOG_INFO << "websocket connection closed: " << reason;
          std::lock_guard<std::mutex> _(mtx);
          users.erase(&conn);
      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
          // Expected JSON: {"canId": "0x100", "data": "45"}
          auto j = crow::json::load(data);
          if (!j) return;
          
          if (j.has("canId") && j.has("data")) {
              std::string canId = j["canId"].s();
              std::string canData = j["data"].s();
              
              if (canSim.processMessage(canId, canData)) {
                  // Broadcast new state
                  VehicleState state = canSim.getState();
                  crow::json::wvalue res;
                  res["type"] = "STATE";
                  res["steering"] = state.steeringAngle;
                  res["brake"] = state.brakePressure;
                  res["throttle"] = state.throttlePosition;
                  res["speed"] = state.speed;
                  res["rpm"] = state.rpm;
                  res["gear"] = state.gear;
                  
                  std::string broadcast_str = res.dump();
                  
                  std::lock_guard<std::mutex> _(mtx);
                  for (auto u : users) {
                      u->send_text(broadcast_str);
                  }
              }
          }
      });

    // Handle CORS since HTML might be opened locally
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
      .global()
      .headers("*")
      .methods("POST"_method, "GET"_method, "OPTIONS"_method)
      .prefix("*")
      .origin("*");

    std::cout << "Starting Server on port 18080..." << std::endl;
    app.port(18080).multithreaded().run();
}
