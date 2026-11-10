#ifndef PATIENT_H
#define PATIENT_H

#include <string>

class Patient {
public:
    enum class Status { Waiting, Admitted, Discharged };

    Patient(int id, std::string name, int severity, std::string specialty);

    int id() const;
    const std::string &name() const;
    int severity() const;
    const std::string &specialty() const;
    Status status() const;
    std::string statusText() const;
    int bedId() const;
    int doctorId() const;

    void admit(int bed_id, int doctor_id);
    void discharge();

private:
    int id_;
    std::string name_;
    int severity_;
    std::string specialty_;
    Status status_ = Status::Waiting;
    int bed_id_ = 0;
    int doctor_id_ = 0;
};

#endif
