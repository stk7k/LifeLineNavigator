#ifndef DOCTOR_H
#define DOCTOR_H

#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "doctorList.h"

class Doctor {
public:
    Doctor(int id, std::string name, std::string specialty);

    int id() const;
    const std::string &name() const;
    const std::string &specialty() const;
    bool isFree() const;
    int patientId() const;

    void assign(int patient_id);
    void release();

private:
    int id_;
    std::string name_;
    std::string specialty_;
    int patient_id_ = 0;
};

class DoctorRoster {
public:
    DoctorRoster();
    ~DoctorRoster();
    DoctorRoster(const DoctorRoster &) = delete;
    DoctorRoster &operator=(const DoctorRoster &) = delete;

    void add(int id, std::string name, std::string specialty);
    int count() const;
    Doctor *findById(int id) const;
    Doctor *findFree(const std::string &specialty) const;
    int assignFree(const std::string &specialty, int patient_id);
    bool releaseById(int doctor_id);
    void print(std::ostream &os) const;
    std::vector<const Doctor *> all() const;

private:
    DoctorList list_;
    std::vector<std::unique_ptr<Doctor>> doctors_;
};

#endif
