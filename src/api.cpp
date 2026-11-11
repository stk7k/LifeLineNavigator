#include "hospital_service.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <cstdlib>

using nlohmann::json;

static void sendJson(httplib::Response &res, const json &payload, int status = 200) {
    res.status = status;
    res.set_content(payload.dump(), "application/json; charset=utf-8");
}

int main() {
    HospitalService hospital;
    httplib::Server server;

    server.Get("/api/health", [](const httplib::Request &, httplib::Response &res) {
        sendJson(res, {{"success", true}, {"service", "LifeLine Navigator API"}});
    });
    server.Get("/api/dashboard", [&](const httplib::Request &, httplib::Response &res) {
        sendJson(res, hospital.dashboard());
    });
    server.Get("/api/patients", [&](const httplib::Request &, httplib::Response &res) {
        sendJson(res, hospital.patients());
    });
    server.Get("/api/doctors", [&](const httplib::Request &, httplib::Response &res) {
        sendJson(res, hospital.doctors());
    });
    server.Get("/api/beds", [&](const httplib::Request &, httplib::Response &res) {
        sendJson(res, hospital.beds());
    });
    server.Get(R"(/api/bills/(\d+))", [&](const httplib::Request &req, httplib::Response &res) {
        json result = hospital.bill(std::stoi(req.matches[1]));
        sendJson(res, result, result.value("success", true) ? 200 : 404);
    });
    server.Post("/api/patients", [&](const httplib::Request &req, httplib::Response &res) {
        try {
            json body = json::parse(req.body);
            json result = hospital.registerPatient(body);
            sendJson(res, result, result.value("success", false) ? 201 : 400);
        } catch (const std::exception &e) {
            sendJson(res, {{"success", false}, {"message", std::string("Invalid request: ") + e.what()}}, 400);
        }
    });
    server.Post("/api/admissions/next", [&](const httplib::Request &, httplib::Response &res) {
        json result = hospital.admitNext();
        sendJson(res, result, result.value("success", false) ? 200 : 409);
    });
    server.Post(R"(/api/bills/(\d+)/charges)", [&](const httplib::Request &req, httplib::Response &res) {
        try {
            json result = hospital.addCharge(std::stoi(req.matches[1]), json::parse(req.body));
            sendJson(res, result, result.value("success", false) ? 200 : 400);
        } catch (const std::exception &e) {
            sendJson(res, {{"success", false}, {"message", std::string("Invalid request: ") + e.what()}}, 400);
        }
    });
    server.Post(R"(/api/patients/(\d+)/discharge)", [&](const httplib::Request &req, httplib::Response &res) {
        json result = hospital.discharge(std::stoi(req.matches[1]));
        sendJson(res, result, result.value("success", false) ? 200 : 409);
    });

    server.set_error_handler([](const httplib::Request &, httplib::Response &res) {
        if (res.body.empty()) sendJson(res, {{"success", false}, {"message", "The requested resource was not found."}}, 404);
    });
    server.set_mount_point("/", "frontend");

    //const char *host = "127.0.0.1";
    //const int port = 8080;
    //std::cout << "LifeLine Navigator is running at http://" << host << ":" << port << "\n";
    //std::cout << "Press Ctrl+C to stop the server.\n";
    //if (!server.listen(host, port)) {
    //    std::cerr << "Could not start the server. Is port " << port << " already in use?\n";
    //    return 1;
    //}

    // RENDER SERVER CONNECTION
    const char *host = "0.0.0.0";
    int port = 8080;

    if (const char *port_env = std::getenv("PORT")) {
        port = std::stoi(port_env);
    }

    std::cout << "LifeLine Navigator listening on "
            << host << ":" << port << "\n";

    if (!server.listen(host, port)) {
        std::cerr << "Could not start server.\n";
        return 1;
    }
    return 0;
}
