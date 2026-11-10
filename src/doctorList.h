#ifndef DOCTORLIST_H
#define DOCTORLIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DoctorNode {
    void *doctor;
    struct DoctorNode *next;
} DoctorNode;

typedef struct {
    DoctorNode *head;
    int count;
} DoctorList;

typedef int (*DoctorMatch)(void *doctor, void *context);
typedef void (*DoctorVisit)(void *doctor, void *context);

void dlist_init(DoctorList *list);
int dlist_append(DoctorList *list, void *doctor);
void *dlist_find(const DoctorList *list, DoctorMatch match, void *context);
void dlist_for_each(const DoctorList *list, DoctorVisit visit, void *context);
int dlist_count(const DoctorList *list);
void dlist_destroy(DoctorList *list);

#ifdef __cplusplus
}
#endif

#endif
