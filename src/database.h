#ifndef DATABASE_H
#define DATABASE_H

#include <libpq-fe.h>
#include <string>

class Database {
public:
    Database();
    ~Database();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    bool connected() const;
    std::string error() const;

private:
    PGconn *connection_ = nullptr;
};

#endif