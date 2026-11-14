#include "database.h"

#include <cstdlib>
#include <stdexcept>

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