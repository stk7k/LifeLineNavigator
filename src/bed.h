#ifndef BED_H
#define BED_H

#include <vector>
#include "bedStack.h"

enum class WardType { General, ICU };

class Bed {
public:
    Bed(int id, WardType ward);

    int id() const;
    WardType ward() const;
    bool isOccupied() const;
    int patientId() const;

    void assign(int patient_id);
    void release();

private:
    int id_;
    WardType ward_;
    int patient_id_ = 0;
};

class WardBeds {
public:
    WardBeds(WardType ward, int first_id, int count);
    ~WardBeds();
    WardBeds(const WardBeds &) = delete;
    WardBeds &operator=(const WardBeds &) = delete;

    int allocate(int patient_id);
    bool release(int bed_id);
    bool holds(int bed_id) const;
    int freeCount() const;
    const std::vector<Bed> &beds() const;

private:
    Bed *findBed(int bed_id);
    std::vector<Bed> beds_;
    BedStack stack_;
};

#endif
