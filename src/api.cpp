#include <cctype>
#include "hospital_service.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include "database.h"
#include <stdexcept>
#include "billing.h"

using nlohmann::json;

static void sendJson(httplib::Response &res, const json &payload, int status = 200) {
    res.status = status;
    res.set_content(payload.dump(), "application/json; charset=utf-8");
}

int main() {
    Database database;
    HospitalService hospital;
    httplib::Server server;

    server.Get("/api/health", [](const httplib::Request &, httplib::Response &res) {
        sendJson(res, {{"success", true}, {"service", "LifeLine Navigator API"}});
    });


server.Post("/api/auth/signup", [&](const httplib::Request &req,
                                    httplib::Response &res) {
    bool transactionStarted = false;

    try {
        json body = json::parse(req.body);

        std::string name = body.at("name").get<std::string>();
        std::string username = body.at("username").get<std::string>();
        std::string password = body.at("password").get<std::string>();

        // Basic input validation
        if (name.empty() || name.size() > 80 ||
            username.size() < 3 || username.size() > 30 ||
            password.size() < 8) {
            sendJson(res, {
                {"success", false},
                {"message", "Please check your registration details."}
            }, 400);
            return;
        }

        // Normalise username
        for (char &c : username) {
            c = static_cast<char>(
                std::tolower(static_cast<unsigned char>(c))
            );
        }

        for (unsigned char c : username) {
            if (!std::isalnum(c) && c != '_') {
                sendJson(res, {
                    {"success", false},
                    {"message", "Username can contain only letters, numbers and underscores."}
                }, 400);
                return;
            }
        }

        // Begin database transaction
        PGresult *result = database.execute("BEGIN");
        PQclear(result);
        transactionStarted = true;

        // Create patient and retrieve Supabase-generated ID
        result = database.execute(
            "INSERT INTO public.patients "
            "(name, severity, specialty, status) "
            "VALUES ($1, 5, 'General Medicine', 'waiting') "
            "RETURNING id",
            {name}
        );

        int patientId = std::stoi(PQgetvalue(result, 0, 0));
        PQclear(result);

        // Create login account with bcrypt password hash
        result = database.execute(
            "INSERT INTO public.patient_accounts "
            "(patient_id, username, password_hash) "
            "VALUES ($1, $2, crypt($3, gen_salt('bf', 12)))",
            {
                std::to_string(patientId),
                username,
                password
            }
        );
        PQclear(result);

        // Commit both inserts together
        result = database.execute("COMMIT");
        PQclear(result);
        transactionStarted = false;

        // Add patient to the existing in-memory hospital queue
        json patientBody = {
            {"name", name},
            {"severity", 5},
            {"specialty", "General Medicine"}
        };

        json registration =
            hospital.registerPatientWithId(patientId, patientBody);

        if (!registration.value("success", false)) {
            // Compensate if the in-memory registration fails
            result = database.execute(
                "DELETE FROM public.patients WHERE id = $1",
                {std::to_string(patientId)}
            );
            PQclear(result);

            sendJson(res, registration, 500);
            return;
        }

        sendJson(res, {
            {"success", true},
            {"message", "Account created successfully."},
            {"patientId", patientId}
        }, 201);

    } catch (const std::exception &e) {

        if (transactionStarted) {
            try {
                PGresult *rollback = database.execute("ROLLBACK");
                PQclear(rollback);
            } catch (...) {
                // Preserve the original error.
            }
        }

        std::cerr << "Signup error: " << e.what() << "\n";

        sendJson(res, {
            {"success", false},
            {"message", "Registration failed. Please check your details or try again."}
        }, 400);
    }
});


    server.Post("/api/auth/login", [&](const httplib::Request &req,
                                   httplib::Response &res) {
    try {
        json body = json::parse(req.body);

        std::string username = body.at("username").get<std::string>();
        std::string password = body.at("password").get<std::string>();

        // Normalise username
        for (char &c : username) {
            c = static_cast<char>(
                std::tolower(static_cast<unsigned char>(c))
            );
        }

        if (username.empty() || password.empty()) {
            sendJson(res, {
                {"success", false},
                {"message", "Please enter your username and password."}
            }, 400);
            return;
        }

        // Verify credentials and retrieve the patient's details
        PGresult *result = database.execute(
            "SELECT p.id, p.name "
            "FROM public.patient_accounts pa "
            "JOIN public.patients p ON p.id = pa.patient_id "
            "WHERE pa.username = $1 "
            "AND pa.password_hash = crypt($2, pa.password_hash)",
            {username, password}
        );

        if (PQntuples(result) == 0) {
            PQclear(result);

            sendJson(res, {
                {"success", false},
                {"message", "Invalid username or password."}
            }, 401);
            return;
        }

        int patientId = std::stoi(PQgetvalue(result, 0, 0));
        std::string patientName = PQgetvalue(result, 0, 1);

        PQclear(result);

        sendJson(res, {
            {"success", true},
            {"message", "Login successful."},
            {"patientId", patientId},
            {"name", patientName}
        });

    } catch (const std::exception &e) {
        std::cerr << "Login error: " << e.what() << "\n";

        sendJson(res, {
            {"success", false},
            {"message", "Login failed. Please try again."}
        }, 400);
    }
});


    server.Get(R"(/api/patients/(\d+))",
    [&](const httplib::Request &req, httplib::Response &res) {
        try {
            int patientId = std::stoi(req.matches[1]);

            PGresult *result = database.execute(
                "SELECT "
                "p.id, p.name, p.severity, p.specialty, p.status, "
                "COALESCE(p.bed_id, 0), "
                "COALESCE(p.doctor_id, 0), "
                "COALESCE(d.name, ''), "
                "COALESCE(d.specialty, ''), "
                "COALESCE(b.ward, ''), "
                "p.admitted_at "
                "FROM public.patients p "
                "LEFT JOIN public.doctors d ON d.id = p.doctor_id "
                "LEFT JOIN public.beds b ON b.id = p.bed_id "
                "WHERE p.id = $1",
                {std::to_string(patientId)}
            );

            if (PQntuples(result) == 0) {
                PQclear(result);

                sendJson(res, {
                    {"success", false},
                    {"message", "Patient record not found."}
                }, 404);

                return;
            }

            std::string status = PQgetvalue(result, 0, 4);

            // Format status for the frontend
            if (status == "waiting") status = "Waiting";
            else if (status == "admitted") status = "Admitted";
            else if (status == "discharged") status = "Discharged";

            json patient = {
                {"id", std::stoi(PQgetvalue(result, 0, 0))},
                {"name", PQgetvalue(result, 0, 1)},
                {"severity", std::stoi(PQgetvalue(result, 0, 2))},
                {"specialty", PQgetvalue(result, 0, 3)},
                {"status", status},
                {"bedId", std::stoi(PQgetvalue(result, 0, 5))},
                {"doctorId", std::stoi(PQgetvalue(result, 0, 6))},
                {"doctorName", PQgetvalue(result, 0, 7)},
                {"doctorSpecialty", PQgetvalue(result, 0, 8)},
                {"bedWard", PQgetvalue(result, 0, 9)},
                {"admissionDate", nullptr}
            };

            if (!PQgetisnull(result, 0, 10)) {
                patient["admissionDate"] = PQgetvalue(result, 0, 10);
            }

            PQclear(result);

            sendJson(res, {
                {"success", true},
                {"patient", patient}
            });

        } catch (const std::exception &e) {
            std::cerr << "Patient lookup error: " << e.what() << "\n";

            sendJson(res, {
                {"success", false},
                {"message", "Could not retrieve patient information."}
            }, 500);
        }
    }
);


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
    try {
        // Admit patient using existing C++ logic
        json result = hospital.admitNext();

        if (!result.value("success", false)) {
            sendJson(res, result, 409);
            return;
        }

        // Extract admission details
        const json &patient = result.at("patient");

        int patientId = patient.at("id").get<int>();
        int bedId = patient.at("bedId").get<int>();
        int doctorId = patient.at("doctorId").get<int>();

        // Start database transaction
        PGresult *dbResult = database.execute("BEGIN");
        PQclear(dbResult);

        try {
            // Update patient record
            dbResult = database.execute(
                "UPDATE public.patients "
                "SET status = 'admitted', "
                "bed_id = $2, "
                "doctor_id = $3, "
                "admitted_at = NOW() "
                "WHERE id = $1",
                {
                    std::to_string(patientId),
                    std::to_string(bedId),
                    std::to_string(doctorId)
                }
            );
            PQclear(dbResult);

            // Mark bed as occupied
            dbResult = database.execute(
                "UPDATE public.beds "
                "SET is_occupied = TRUE "
                "WHERE id = $1",
                {std::to_string(bedId)}
            );
            PQclear(dbResult);

            // Mark doctor as unavailable
            dbResult = database.execute(
                "UPDATE public.doctors "
                "SET is_available = FALSE "
                "WHERE id = $1",
                {std::to_string(doctorId)}
            );
            PQclear(dbResult);

            // Determine the admission charge using the assigned bed's ward
dbResult = database.execute(
    "SELECT ward FROM public.beds WHERE id = $1",
    {std::to_string(bedId)}
);

if (PQntuples(dbResult) != 1) {
    PQclear(dbResult);
    throw std::runtime_error("Assigned bed not found in database.");
}

std::string ward = PQgetvalue(dbResult, 0, 0);
PQclear(dbResult);

long long bedCharge =
    (ward == "ICU") ? ICU_BED_FEE : GENERAL_BED_FEE;

long long consultationCharge = CONSULT_FEE;
long long totalPaise = bedCharge + consultationCharge;

// Create or refresh the patient's bill
dbResult = database.execute(
    "INSERT INTO public.bills (patient_id, closed, total_paise) "
    "VALUES ($1, FALSE, $2) "
    "ON CONFLICT (patient_id) DO UPDATE "
    "SET closed = FALSE, total_paise = EXCLUDED.total_paise "
    "RETURNING id",
    {
        std::to_string(patientId),
        std::to_string(totalPaise)
    }
);

if (PQntuples(dbResult) != 1) {
    PQclear(dbResult);
    throw std::runtime_error("Could not create patient's bill.");
}

long long billId = std::stoll(PQgetvalue(dbResult, 0, 0));
PQclear(dbResult);

// Remove previous items if this bill already existed
dbResult = database.execute(
    "DELETE FROM public.bill_items WHERE bill_id = $1",
    {std::to_string(billId)}
);
PQclear(dbResult);

// Insert admission charge
dbResult = database.execute(
    "INSERT INTO public.bill_items "
    "(bill_id, description, amount_paise) "
    "VALUES ($1, $2, $3)",
    {
        std::to_string(billId),
        ward + " bed (admission)",
        std::to_string(bedCharge)
    }
);
PQclear(dbResult);

// Insert consultation charge
dbResult = database.execute(
    "INSERT INTO public.bill_items "
    "(bill_id, description, amount_paise) "
    "VALUES ($1, $2, $3)",
    {
        std::to_string(billId),
        "Doctor consultation",
        std::to_string(consultationCharge)
    }
);
PQclear(dbResult);

            // Commit changes
            dbResult = database.execute("COMMIT");
            PQclear(dbResult);

        } catch (...) {
            try {
                dbResult = database.execute("ROLLBACK");
                PQclear(dbResult);
            } catch (...) {}

            throw;
        }

        sendJson(res, result);

    } catch (const std::exception &e) {
        std::cerr << "Admission database error: " << e.what() << "\n";

        sendJson(res, {
            {"success", false},
            {"message", "Patient admission could not be synchronized with the database."}
        }, 500);
    }
});


   server.Post(R"(/api/patients/(\d+)/discharge)",
    [&](const httplib::Request &req, httplib::Response &res) {

        bool transactionStarted = false;

        try {
            int patientId = std::stoi(req.matches[1]);

            // Perform discharge using existing C++ logic
            json dischargeResult = hospital.discharge(patientId);

            if (!dischargeResult.value("success", false)) {
                sendJson(res, dischargeResult, 409);
                return;
            }

            long long totalPaise =
                dischargeResult.at("totalPaise").get<long long>();

            // Begin database transaction
            PGresult *dbResult = database.execute("BEGIN");
            PQclear(dbResult);
            transactionStarted = true;

            // Retrieve assigned bed and doctor
            dbResult = database.execute(
                "SELECT bed_id, doctor_id "
                "FROM public.patients "
                "WHERE id = $1",
                {std::to_string(patientId)}
            );

            if (PQntuples(dbResult) == 0) {
                PQclear(dbResult);
                throw std::runtime_error("Patient record not found in database.");
            }

            int bedId = PQgetisnull(dbResult, 0, 0)
                ? 0 : std::stoi(PQgetvalue(dbResult, 0, 0));

            int doctorId = PQgetisnull(dbResult, 0, 1)
                ? 0 : std::stoi(PQgetvalue(dbResult, 0, 1));

            PQclear(dbResult);

            // Update patient status
            dbResult = database.execute(
                "UPDATE public.patients "
                "SET status = 'discharged', "
                "discharged_at = NOW() "
                "WHERE id = $1",
                {std::to_string(patientId)}
            );
            PQclear(dbResult);

            // Release bed
            if (bedId > 0) {
                dbResult = database.execute(
                    "UPDATE public.beds "
                    "SET is_occupied = FALSE "
                    "WHERE id = $1",
                    {std::to_string(bedId)}
                );
                PQclear(dbResult);
            }

            // Make doctor available again
            if (doctorId > 0) {
                dbResult = database.execute(
                    "UPDATE public.doctors "
                    "SET is_available = TRUE "
                    "WHERE id = $1",
                    {std::to_string(doctorId)}
                );
                PQclear(dbResult);
            }

            // Finalize the patient's bill
            dbResult = database.execute(
                "UPDATE public.bills "
                "SET closed = TRUE, total_paise = $2 "
                "WHERE patient_id = $1",
                {
                    std::to_string(patientId),
                    std::to_string(totalPaise)
                }
            );

            if (std::string(PQcmdTuples(dbResult)) != "1") {
                PQclear(dbResult);
                throw std::runtime_error("Could not finalize the patient's bill.");
            }

            PQclear(dbResult);

            // Commit all database changes
            dbResult = database.execute("COMMIT");
            PQclear(dbResult);

            transactionStarted = false;

            sendJson(res, dischargeResult);

        } catch (const std::exception &e) {

            if (transactionStarted) {
                try {
                    PGresult *rollback = database.execute("ROLLBACK");
                    PQclear(rollback);
                } catch (...) {}
            }

            std::cerr << "Discharge database error: "
                      << e.what() << "\n";

            sendJson(res, {
                {"success", false},
                {"message", "Discharge could not be synchronized with the database."}
            }, 500);
        }
    }
);
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
