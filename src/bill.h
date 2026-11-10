#ifndef BILL_H
#define BILL_H

#include <ostream>
#include <string>
#include <vector>
#include "bed.h"
#include "billing.h"
#include "doctor.h"
#include "patient.h"

class Bill {
public:
    Bill(int patient_id, std::string patient_name);
    ~Bill();
    Bill(const Bill &) = delete;
    Bill &operator=(const Bill &) = delete;

    int patientId() const;
    void addCharge(const std::string &description, long long amount_paise);
    long long total() const;
    bool isClosed() const;
    const ChargeList &charges() const;
    long long close();
    void print(std::ostream &os) const;

private:
    int patient_id_;
    std::string patient_name_;
    ChargeList charges_;
    bool closed_ = false;
};

std::string formatPaise(long long paise);

long long dischargePatient(Patient &patient, Bill &bill, const std::vector<WardBeds *> &wards,
                           DoctorRoster &doctors, std::ostream &receipt);

#endif
