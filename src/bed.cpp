#include "bed.h"
#include <new>
#include <stdexcept>

Bed::Bed(int id, WardType ward) : id_(id), ward_(ward) {
    if (id_ <= 0) {
        throw std::invalid_argument("bed id must be positive");
    }
}

int Bed::id() const {
    return id_;
}

WardType Bed::ward() const {
    return ward_;
}

bool Bed::isOccupied() const {
    return patient_id_ != 0;
}

int Bed::patientId() const {
    return patient_id_;
}

void Bed::assign(int patient_id) {
    if (isOccupied()) {
        throw std::logic_error("bed is already occupied");
    }
    if (patient_id <= 0) {
        throw std::invalid_argument("patient id must be positive");
    }
    patient_id_ = patient_id;
}

void Bed::release() {
    if (!isOccupied()) {
        throw std::logic_error("bed is already free");
    }
    patient_id_ = 0;
}

WardBeds::WardBeds(WardType ward, int first_id, int count) {
    int i;
    if (first_id <= 0 || count <= 0) {
        throw std::invalid_argument("invalid ward range");
    }
    for (i = 0; i < count; i++) {
        beds_.emplace_back(first_id + i, ward);
    }
    if (bedstack_init(&stack_, count) != 0) {
        throw std::bad_alloc();
    }
    for (i = count - 1; i >= 0; i--) {
        bedstack_push(&stack_, first_id + i);
    }
}

WardBeds::~WardBeds() {
    bedstack_destroy(&stack_);
}

Bed *WardBeds::findBed(int bed_id) {
    for (Bed &bed : beds_) {
        if (bed.id() == bed_id) {
            return &bed;
        }
    }
    return nullptr;
}

int WardBeds::allocate(int patient_id) {
    int bed_id = 0;
    if (patient_id <= 0 || bedstack_pop(&stack_, &bed_id) != 0) {
        return -1;
    }
    findBed(bed_id)->assign(patient_id);
    return bed_id;
}

bool WardBeds::release(int bed_id) {
    Bed *bed = findBed(bed_id);
    if (!bed || !bed->isOccupied()) {
        return false;
    }
    bed->release();
    return bedstack_push(&stack_, bed_id) == 0;
}

bool WardBeds::holds(int bed_id) const {
    for (const Bed &bed : beds_) {
        if (bed.id() == bed_id) {
            return true;
        }
    }
    return false;
}

int WardBeds::freeCount() const {
    return bedstack_count(&stack_);
}

const std::vector<Bed> &WardBeds::beds() const {
    return beds_;
}
