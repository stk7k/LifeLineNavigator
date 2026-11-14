
#ifndef DATABASE_H
#define DATABASE_H

#include <libpq-fe.h>
#include <string>
#include <vector>
#include <optional>

class Database {
public:
    Database();
    ~Database();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    bool connected() const;
    std::string error() const;

    // Execute parameterised SQL queries.
    // Caller must release the returned result using PQclear().
    PGresult *execute(
        const std::string &query,
        const std::vector<std::optional<std::string>> &params = {}
    ) const;

private:
    PGconn *connection_ = nullptr;
};

#endif
