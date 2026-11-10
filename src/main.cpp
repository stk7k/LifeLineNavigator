#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "bed.h"
#include "bill.h"
#include "doctor.h"
#include "patient.h"
#include "queue.h"

namespace {

constexpr int ICU_SEVERITY = 8;
constexpr long long GENERAL_BED_FEE = 50000;
constexpr long long ICU_BED_FEE = 250000;
constexpr long long CONSULT_FEE = 30000;

struct Hospital {
    Hospital() {
        queue_init(&waiting);
        doctors.add(1, "Dr. Iyer", "General Medicine");
        doctors.add(2, "Dr. Mehta", "General Medicine");
        doctors.add(3, "Dr. Rao", "Cardiology");
        doctors.add(4, "Dr. Singh", "Critical Care");
    }
    ~Hospital() {
        queue_destroy(&waiting);
    }
    Hospital(const Hospital &) = delete;
    Hospital &operator=(const Hospital &) = delete;

    Queue waiting;
    std::map<int, std::unique_ptr<Patient>> patients;
    std::map<int, std::unique_ptr<Bill>> bills;
    WardBeds general{WardType::General, 1, 3};
    WardBeds icu{WardType::ICU, 4, 2};
    DoctorRoster doctors;
    int next_patient_id = 1;
};

std::string readLine(const std::string &prompt) {
    std::string line;
    std::cout << prompt;
    if (!std::getline(std::cin, line)) {
        std::exit(0);
    }
    return line;
}

int readInt(const std::string &prompt, int lo, int hi) {
    for (;;) {
        std::string line = readLine(prompt);
        try {
            int value = std::stoi(line);
            if (value >= lo && value <= hi) {
                return value;
            }
        } catch (const std::exception &) {
        }
        std::cout << "Please enter a number from " << lo << " to " << hi << ".\n";
    }
}

std::string readText(const std::string &prompt) {
    for (;;) {
        std::string text = readLine(prompt);
        if (!text.empty()) {
            return text;
        }
        std::cout << "This cannot be empty.\n";
    }
}

void registerPatient(Hospital &h) {
    std::string name = readText("Patient name: ");
    int severity = readInt("Severity (1 = mild, 10 = critical): ", 1, 10);
    std::string specialty = readText("Specialty needed (General Medicine, Cardiology, Critical Care): ");
    int id = h.next_patient_id++;
    auto patient = std::make_unique<Patient>(id, name, severity, specialty);
    Patient *raw = patient.get();
    if (queue_enqueue(&h.waiting, raw) != 0) {
        throw std::runtime_error("could not add patient to the queue");
    }
    h.patients[id] = std::move(patient);
    std::cout << "Registered patient #" << id << ". Patients waiting: " << queue_size(&h.waiting) << "\n";
}

void admitNext(Hospital &h) {
    Patient *p = static_cast<Patient *>(queue_peek(&h.waiting));
    if (!p) {
        std::cout << "No patients are waiting.\n";
        return;
    }
    bool icu = p->severity() >= ICU_SEVERITY;
    WardBeds &ward = icu ? h.icu : h.general;
    int bed_id = ward.allocate(p->id());
    if (bed_id < 0) {
        std::cout << "No " << (icu ? "ICU" : "general") << " bed is free. " << p->name() << " stays in the queue.\n";
        return;
    }
    int doctor_id = h.doctors.assignFree(p->specialty(), p->id());
    if (doctor_id < 0) {
        ward.release(bed_id);
        std::cout << "No " << p->specialty() << " doctor is free. " << p->name() << " stays in the queue.\n";
        return;
    }
    queue_dequeue(&h.waiting);
    p->admit(bed_id, doctor_id);
    auto bill = std::make_unique<Bill>(p->id(), p->name());
    bill->addCharge(icu ? "ICU bed (admission)" : "General bed (admission)", icu ? ICU_BED_FEE : GENERAL_BED_FEE);
    bill->addCharge("Doctor consultation", CONSULT_FEE);
    std::cout << p->name() << " admitted to bed " << bed_id << " with doctor #" << doctor_id << ".\n";
    h.bills[p->id()] = std::move(bill);
}

void addCharge(Hospital &h) {
    int id = readInt("Patient id: ", 1, 1000000);
    auto found = h.bills.find(id);
    if (found == h.bills.end()) {
        throw std::logic_error("no bill exists for that patient");
    }
    std::string description = readText("Charge description: ");
    int rupees = readInt("Amount in rupees: ", 0, 1000000);
    found->second->addCharge(description, rupees * 100LL);
    std::cout << "Charge added.\n";
}

void discharge(Hospital &h) {
    int id = readInt("Patient id: ", 1, 1000000);
    auto p = h.patients.find(id);
    auto b = h.bills.find(id);
    if (p == h.patients.end() || b == h.bills.end()) {
        throw std::logic_error("unknown patient");
    }
    std::vector<WardBeds *> wards{&h.general, &h.icu};
    dischargePatient(*p->second, *b->second, wards, h.doctors, std::cout);
}

}

extern "C" void printWaiting(void *item, void *) {
    const Patient *p = static_cast<const Patient *>(item);
    std::cout << "  #" << p->id() << " " << p->name() << " (severity " << p->severity()
              << ", needs " << p->specialty() << ")\n";
}

namespace {

void showStatus(const Hospital &h) {
    std::cout << "Waiting (" << queue_size(&h.waiting) << "):\n";
    queue_for_each(&h.waiting, printWaiting, nullptr);
    std::cout << "Free beds: general " << h.general.freeCount() << ", ICU " << h.icu.freeCount() << "\n";
    std::cout << "Doctors:\n";
    h.doctors.print(std::cout);
}

void printMenu() {
    std::cout << "\n1. Register patient\n2. Admit next waiting patient\n3. Add charge to a bill\n"
              << "4. Discharge patient\n5. Show status\n6. Exit\n";
}

}

int main() {
    Hospital hospital;
    for (;;) {
        printMenu();
        int choice = readInt("Choose an option: ", 1, 6);
        if (choice == 6) {
            break;
        }
        try {
            switch (choice) {
            case 1:
                registerPatient(hospital);
                break;
            case 2:
                admitNext(hospital);
                break;
            case 3:
                addCharge(hospital);
                break;
            case 4:
                discharge(hospital);
                break;
            case 5:
                showStatus(hospital);
                break;
            }
        } catch (const std::exception &e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
    std::cout << "Goodbye.\n";
    return 0;
}
