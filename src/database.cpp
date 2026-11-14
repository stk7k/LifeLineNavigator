
#include "database.h"

#include <cstdlib>
#include <stdexcept>
#include <vector>

Database::Database() {
    const char *connectionString = std::getenv("DATABASE_URL");

    if (!connectionString || !*connectionString) {
        throw std::runtime_error(
            "DATABASE_URL environment variable is not set."
        );
    }

    connection_ = PQconnectdb(connectionString);

    if (!connected()) {
        std::string message = error();

        if (connection_) {
            PQfinish(connection_);
            connection_ = nullptr;
        }

        throw std::runtime_error(
            "Supabase connection failed: " + message
        );
    }
}

Database::~Database() {
    if (connection_) {
        PQfinish(connection_);
    }
}

bool Database::connected() const {
    return connection_ &&
           PQstatus(connection_) == CONNECTION_OK;
}

std::string Database::error() const {
    if (!connection_) {
        return "No database connection.";
    }

    return PQerrorMessage(connection_);
}

PGresult *Database::execute(
    const std::string &query,
    const std::vector<std::optional<std::string>> &params
) const {

    if (!connected()) {
        throw std::runtime_error("Database connection is unavailable.");
    }

    std::vector<const char *> values;
    values.reserve(params.size());

    for (const auto &param : params) {
        values.push_back(param ? param->c_str() : nullptr);
    }

    PGresult *result = PQexecParams(
        connection_,
        query.c_str(),
        static_cast<int>(params.size()),
        nullptr,
        values.empty() ? nullptr : values.data(),
        nullptr,
        nullptr,
        0
    );

    if (!result) {
        throw std::runtime_error(error());
    }

    ExecStatusType status = PQresultStatus(result);

    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        std::string message = PQresultErrorMessage(result);
        PQclear(result);

        throw std::runtime_error(
            "Database query failed: " + message
        );
    }

    return result;
}
