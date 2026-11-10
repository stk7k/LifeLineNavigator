#include "patient.h"
#include <stdexcept>
#include <utility>

Patient::Patient(int id, std::string name, int severity, std::string specialty)
    : id_(id), name_(std::move(name)), severity_(severity), specialty_(std::move(specialty)) {
    if (id_ <= 0) {
        throw std::invalid_argument("patient id must be positive");
    }
    if (name_.empty()) {
        throw std::invalid_argument("name must not be empty");
    }
    if (severity_ < 1 || severity_ > 10) {
        throw std::invalid_argument("severity must be from 1 to 10");
    }
    if (specialty_.empty()) {
        throw std::invalid_argument("specialty must not be empty");
    }
}

int Patient::id() const {
    return id_;
}

const std::string &Patient::name() const {
    return name_;
}

int Patient::severity() const {
    return severity_;
}

const std::string &Patient::specialty() const {
    return specialty_;
}

Patient::Status Patient::status() const {
    return status_;
}

std::string Patient::statusText() const {
    switch (status_) {
    case Status::Waiting:
        return "Waiting";
    case Status::Admitted:
        return "Admitted";
    case Status::Discharged:
        return "Discharged";
    }
    return "Unknown";
}

int Patient::bedId() const {
    return bed_id_;
}

int Patient::doctorId() const {
    return doctor_id_;
}

void Patient::admit(int bed_id, int doctor_id) {
    if (status_ != Status::Waiting) {
        throw std::logic_error("only a waiting patient can be admitted");
    }
    bed_id_ = bed_id;
    doctor_id_ = doctor_id;
    status_ = Status::Admitted;
}

void Patient::discharge() {
    if (status_ != Status::Admitted) {
        throw std::logic_error("only an admitted patient can be discharged");
    }
    status_ = Status::Discharged;
}
