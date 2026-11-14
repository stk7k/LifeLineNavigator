#ifndef HOSPITAL_SERVICE_H
#define HOSPITAL_SERVICE_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "bed.h"
#include "bill.h"
#include "doctor.h"
#include "patient.h"
#include "queue.h"
#include <nlohmann/json.hpp>

class HospitalService {
public:
    HospitalService();
    ~HospitalService();
    HospitalService(const HospitalService &) = delete;
    HospitalService &operator=(const HospitalService &) = delete;

    nlohmann::json dashboard();
    nlohmann::json patients();
    nlohmann::json doctors();
    nlohmann::json beds();
    nlohmann::json bill(int patient_id);
    nlohmann::json registerPatient(const nlohmann::json &body);
    nlohmann::json registerPatientWithId(
        int patient_id,
        const nlohmann::json &body
    );
    nlohmann::json admitNext();
    nlohmann::json addCharge(int patient_id, const nlohmann::json &body);
    nlohmann::json discharge(int patient_id);

private:
    Queue waiting_{};
    std::map<int, std::unique_ptr<Patient>> patients_;
    std::map<int, std::unique_ptr<Bill>> bills_;
    WardBeds general_{WardType::General, 1, 3};
    WardBeds icu_{WardType::ICU, 4, 2};
    DoctorRoster doctors_;
    int next_patient_id_ = 1;
    std::mutex mutex_;
};

#endif
