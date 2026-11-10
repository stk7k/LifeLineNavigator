#include "bill.h"
#include <cstdio>
#include <new>
#include <stdexcept>
#include <utility>

Bill::Bill(int patient_id, std::string patient_name)
    : patient_id_(patient_id), patient_name_(std::move(patient_name)) {
    if (charges_init(&charges_) != 0) {
        throw std::bad_alloc();
    }
}

Bill::~Bill() {
    charges_destroy(&charges_);
}

int Bill::patientId() const {
    return patient_id_;
}

void Bill::addCharge(const std::string &description, long long amount_paise) {
    if (closed_) {
        throw std::logic_error("bill is closed");
    }
    if (description.empty()) {
        throw std::invalid_argument("charge needs a description");
    }
    if (amount_paise < 0) {
        throw std::invalid_argument("charge cannot be negative");
    }
    if (charges_add(&charges_, description.c_str(), amount_paise) != 0) {
        throw std::bad_alloc();
    }
}

long long Bill::total() const {
    return charges_total(&charges_);
}

bool Bill::isClosed() const {
    return closed_;
}

const ChargeList &Bill::charges() const {
    return charges_;
}

long long Bill::close() {
    if (closed_) {
        throw std::logic_error("bill is already closed");
    }
    closed_ = true;
    return total();
}

void Bill::print(std::ostream &os) const {
    int i;
    os << "Bill for patient #" << patient_id_ << " (" << patient_name_ << ")\n";
    for (i = 0; i < charges_count(&charges_); i++) {
        const Charge *c = charges_get(&charges_, i);
        os << "  " << c->description << ": " << formatPaise(c->amount_paise) << "\n";
    }
    os << "  Total: " << formatPaise(total()) << "\n";
}

std::string formatPaise(long long paise) {
    std::string digits = std::to_string(paise / 100);
    std::string grouped;
    char cents[4];
    size_t i;
    for (i = 0; i < digits.size(); i++) {
        if (i > 0 && (digits.size() - i) % 3 == 0) {
            grouped += ',';
        }
        grouped += digits[i];
    }
    std::snprintf(cents, sizeof cents, "%02lld", paise % 100);
    return "Rs " + grouped + "." + cents;
}

long long dischargePatient(Patient &patient, Bill &bill, const std::vector<WardBeds *> &wards,
                           DoctorRoster &doctors, std::ostream &receipt) {
    WardBeds *owner = nullptr;
    long long total;
    for (WardBeds *ward : wards) {
        if (ward->holds(patient.bedId())) {
            owner = ward;
            break;
        }
    }
    if (patient.status() != Patient::Status::Admitted) {
        throw std::logic_error("only an admitted patient can be discharged");
    }
    if (bill.isClosed()) {
        throw std::logic_error("bill is already closed");
    }
    if (!owner) {
        throw std::logic_error("patient's bed was not found");
    }
    total = bill.close();
    owner->release(patient.bedId());
    doctors.releaseById(patient.doctorId());
    patient.discharge();
    receipt << "DISCHARGE RECEIPT\n";
    bill.print(receipt);
    return total;
}
