#include "doctor.h"
#include <new>
#include <stdexcept>
#include <utility>

extern "C" {

static int matchFreeWithSpecialty(void *item, void *context) {
    const Doctor *d = static_cast<const Doctor *>(item);
    const std::string *wanted = static_cast<const std::string *>(context);
    return d->isFree() && d->specialty() == *wanted;
}

static int matchId(void *item, void *context) {
    const Doctor *d = static_cast<const Doctor *>(item);
    return d->id() == *static_cast<const int *>(context);
}

static void printDoctor(void *item, void *context) {
    const Doctor *d = static_cast<const Doctor *>(item);
    std::ostream &os = *static_cast<std::ostream *>(context);
    os << "  Doctor #" << d->id() << " " << d->name() << " (" << d->specialty() << "): ";
    if (d->isFree()) {
        os << "free\n";
    } else {
        os << "with patient #" << d->patientId() << "\n";
    }
}

}

Doctor::Doctor(int id, std::string name, std::string specialty)
    : id_(id), name_(std::move(name)), specialty_(std::move(specialty)) {
    if (id_ <= 0) {
        throw std::invalid_argument("doctor id must be positive");
    }
    if (name_.empty()) {
        throw std::invalid_argument("name must not be empty");
    }
    if (specialty_.empty()) {
        throw std::invalid_argument("specialty must not be empty");
    }
}

int Doctor::id() const {
    return id_;
}

const std::string &Doctor::name() const {
    return name_;
}

const std::string &Doctor::specialty() const {
    return specialty_;
}

bool Doctor::isFree() const {
    return patient_id_ == 0;
}

int Doctor::patientId() const {
    return patient_id_;
}

void Doctor::assign(int patient_id) {
    if (!isFree()) {
        throw std::logic_error("doctor is already with a patient");
    }
    if (patient_id <= 0) {
        throw std::invalid_argument("patient id must be positive");
    }
    patient_id_ = patient_id;
}

void Doctor::release() {
    if (isFree()) {
        throw std::logic_error("doctor is already free");
    }
    patient_id_ = 0;
}

DoctorRoster::DoctorRoster() {
    dlist_init(&list_);
}

DoctorRoster::~DoctorRoster() {
    dlist_destroy(&list_);
}

void DoctorRoster::add(int id, std::string name, std::string specialty) {
    if (findById(id)) {
        throw std::invalid_argument("doctor id already exists");
    }
    doctors_.push_back(std::make_unique<Doctor>(id, std::move(name), std::move(specialty)));
    if (dlist_append(&list_, doctors_.back().get()) != 0) {
        doctors_.pop_back();
        throw std::bad_alloc();
    }
}

int DoctorRoster::count() const {
    return dlist_count(&list_);
}

Doctor *DoctorRoster::findById(int id) const {
    return static_cast<Doctor *>(dlist_find(&list_, matchId, &id));
}

Doctor *DoctorRoster::findFree(const std::string &specialty) const {
    return static_cast<Doctor *>(
        dlist_find(&list_, matchFreeWithSpecialty, const_cast<std::string *>(&specialty)));
}

int DoctorRoster::assignFree(const std::string &specialty, int patient_id) {
    Doctor *d = findFree(specialty);
    if (!d) {
        return -1;
    }
    d->assign(patient_id);
    return d->id();
}

bool DoctorRoster::releaseById(int doctor_id) {
    Doctor *d = findById(doctor_id);
    if (!d || d->isFree()) {
        return false;
    }
    d->release();
    return true;
}

void DoctorRoster::print(std::ostream &os) const {
    dlist_for_each(&list_, printDoctor, &os);
}

std::vector<const Doctor *> DoctorRoster::all() const {
    std::vector<const Doctor *> result;
    result.reserve(doctors_.size());
    for (const auto &doctor : doctors_) result.push_back(doctor.get());
    return result;
}
