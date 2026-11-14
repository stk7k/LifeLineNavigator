#include "hospital_service.h"
#include <sstream>
#include <stdexcept>

namespace {
constexpr int ICU_SEVERITY = 8;
constexpr long long GENERAL_BED_FEE = 50000;
constexpr long long ICU_BED_FEE = 250000;
constexpr long long CONSULT_FEE = 30000;

nlohmann::json patientJson(const Patient &p) {
    return {
        {"id", p.id()}, {"name", p.name()}, {"severity", p.severity()},
        {"specialty", p.specialty()}, {"status", p.statusText()},
        {"bedId", p.bedId()}, {"doctorId", p.doctorId()}
    };
}

nlohmann::json errorJson(const std::string &message) {
    return {{"success", false}, {"message", message}};
}
}

HospitalService::HospitalService() {
    queue_init(&waiting_);
    doctors_.add(1, "Dr. Iyer", "General Medicine");
    doctors_.add(2, "Dr. Mehta", "General Medicine");
    doctors_.add(3, "Dr. Rao", "Cardiology");
    doctors_.add(4, "Dr. Singh", "Critical Care");
}

HospitalService::~HospitalService() {
    queue_destroy(&waiting_);
}

nlohmann::json HospitalService::dashboard() {
    std::lock_guard<std::mutex> lock(mutex_);
    int waiting_count = queue_size(&waiting_);
    int admitted = 0;
    int discharged = 0;
    for (const auto &entry : patients_) {
        if (entry.second->status() == Patient::Status::Admitted) ++admitted;
        if (entry.second->status() == Patient::Status::Discharged) ++discharged;
    }
    return {
        {"totalPatients", patients_.size()}, {"waitingPatients", waiting_count},
        {"admittedPatients", admitted}, {"dischargedPatients", discharged},
        {"generalBedsFree", general_.freeCount()}, {"generalBedsTotal", 3},
        {"icuBedsFree", icu_.freeCount()}, {"icuBedsTotal", 2},
        {"doctorsTotal", doctors_.count()}
    };
}

nlohmann::json HospitalService::patients() {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result = nlohmann::json::array();
    for (const auto &entry : patients_) result.push_back(patientJson(*entry.second));
    return result;
}

nlohmann::json HospitalService::doctors() {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result = nlohmann::json::array();
    // DoctorRoster::print is intentionally not parsed; its linked-list-backed
    // collection is exposed through all() for a structured API response.
    for (const Doctor *doctor : doctors_.all()) {
        result.push_back({{"id", doctor->id()}, {"name", doctor->name()},
                          {"specialty", doctor->specialty()}, {"available", doctor->isFree()},
                          {"patientId", doctor->patientId()}});
    }
    return result;
}

nlohmann::json HospitalService::beds() {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result = nlohmann::json::array();
    auto append = [&result](const WardBeds &ward, const char *name) {
        for (const Bed &bed : ward.beds()) {
            result.push_back({{"id", bed.id()}, {"ward", name},
                              {"occupied", bed.isOccupied()}, {"patientId", bed.patientId()}});
        }
    };
    append(general_, "General");
    append(icu_, "ICU");
    return result;
}

nlohmann::json HospitalService::bill(int patient_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto found = bills_.find(patient_id);
    auto patient = patients_.find(patient_id);
    if (found == bills_.end() || patient == patients_.end()) return errorJson("Bill not found for this patient.");
    const Bill &bill = *found->second;
    nlohmann::json items = nlohmann::json::array();
    const ChargeList &charges = bill.charges();
    for (int i = 0; i < charges_count(&charges); ++i) {
        const Charge *charge = charges_get(&charges, i);
        items.push_back({{"description", charge->description}, {"amountPaise", charge->amount_paise}});
    }
    return {{"success", true}, {"patient", patientJson(*patient->second)},
            {"closed", bill.isClosed()}, {"totalPaise", bill.total()}, {"items", items}};
}

nlohmann::json HospitalService::registerPatient(const nlohmann::json &body) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        std::string name = body.at("name").get<std::string>();
        std::string specialty = body.at("specialty").get<std::string>();
        int severity = body.at("severity").get<int>();
        if (name.empty() || specialty.empty()) return errorJson("Name and specialty are required.");
        int id = next_patient_id_;
        auto patient = std::make_unique<Patient>(id, name, severity, specialty);
        Patient *raw = patient.get();
        if (queue_enqueue(&waiting_, raw) != 0) return errorJson("Could not add patient to the waiting queue.");
        patients_[id] = std::move(patient);
        ++next_patient_id_;
        return {{"success", true}, {"message", "Patient registered and added to the waiting queue."},
                {"patient", patientJson(*raw)}};
    } catch (const std::exception &e) {
        return errorJson(e.what());
    }
}
// //////


nlohmann::json HospitalService::registerPatientWithId(
    int patient_id,
    const nlohmann::json &body
) 
{
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        if (patient_id <= 0) {
            return errorJson("Invalid patient ID.");
        }

        if (patients_.count(patient_id)) {
            return errorJson("Patient ID already exists.");
        }

        std::string name = body.at("name").get<std::string>();
        std::string specialty = body.at("specialty").get<std::string>();
        int severity = body.at("severity").get<int>();

        auto patient = std::make_unique<Patient>(
            patient_id,
            name,
            severity,
            specialty
        );

        Patient *raw = patient.get();

        // Store the patient before adding it to the queue.
        auto inserted = patients_.emplace(patient_id, std::move(patient));

        if (!inserted.second) {
            return errorJson("Could not register patient.");
        }

        // Add the patient to the existing C waiting queue.
        if (queue_enqueue(&waiting_, raw) != 0) {
            patients_.erase(patient_id);
            return errorJson("Could not add patient to the waiting queue.");
        }

        // Keep future locally generated IDs above this ID.
        if (patient_id >= next_patient_id_) {
            next_patient_id_ = patient_id + 1;
        }

        return {
            {"success", true},
            {"message", "Patient registered and added to the waiting queue."},
            {"patient", patientJson(*raw)}
        };

    } catch (const std::exception &e) {
        return errorJson(e.what());
    }
}


// //////
nlohmann::json HospitalService::admitNext() {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        Patient *patient = static_cast<Patient *>(queue_peek(&waiting_));
        if (!patient) return errorJson("There are no patients waiting for admission.");
        bool needs_icu = patient->severity() >= ICU_SEVERITY;
        WardBeds &ward = needs_icu ? icu_ : general_;
        int bed_id = ward.allocate(patient->id());
        if (bed_id < 0) return errorJson(std::string("No ") + (needs_icu ? "ICU" : "general") + " bed is available.");
        int doctor_id = doctors_.assignFree(patient->specialty(), patient->id());
        if (doctor_id < 0) {
            ward.release(bed_id);
            return errorJson("No available doctor matches this patient's specialty.");
        }
        queue_dequeue(&waiting_);
        patient->admit(bed_id, doctor_id);
        auto new_bill = std::make_unique<Bill>(patient->id(), patient->name());
        new_bill->addCharge(needs_icu ? "ICU bed (admission)" : "General bed (admission)",
                            needs_icu ? ICU_BED_FEE : GENERAL_BED_FEE);
        new_bill->addCharge("Doctor consultation", CONSULT_FEE);
        bills_[patient->id()] = std::move(new_bill);
        return {{"success", true}, {"message", patient->name() + " admitted successfully."},
                {"patient", patientJson(*patient)}};
    } catch (const std::exception &e) {
        return errorJson(e.what());
    }
}

nlohmann::json HospitalService::addCharge(int patient_id, const nlohmann::json &body) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        auto found = bills_.find(patient_id);
        if (found == bills_.end()) return errorJson("No bill exists for this patient.");
        std::string description = body.at("description").get<std::string>();
        int rupees = body.at("amountRupees").get<int>();
        if (rupees < 0 || rupees > 1000000) return errorJson("Amount must be between 0 and 1,000,000 rupees.");
        found->second->addCharge(description, static_cast<long long>(rupees) * 100LL);
        return {{"success", true}, {"message", "Charge added to the bill."}, {"totalPaise", found->second->total()}};
    } catch (const std::exception &e) {
        return errorJson(e.what());
    }
}

nlohmann::json HospitalService::discharge(int patient_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        auto patient = patients_.find(patient_id);
        auto bill = bills_.find(patient_id);
        if (patient == patients_.end() || bill == bills_.end()) return errorJson("Patient or bill not found.");
        std::vector<WardBeds *> wards{&general_, &icu_};
        std::ostringstream receipt;
        long long total = dischargePatient(*patient->second, *bill->second, wards, doctors_, receipt);
        return {{"success", true}, {"message", "Patient discharged successfully."},
                {"totalPaise", total}, {"receipt", receipt.str()}, {"patient", patientJson(*patient->second)}};
    } catch (const std::exception &e) {
        return errorJson(e.what());
    }
}
