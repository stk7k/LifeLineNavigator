#include <stdlib.h>
#include "doctorList.h"

void dlist_init(DoctorList *list) {
    list->head = NULL;
    list->count = 0;
}

int dlist_append(DoctorList *list, void *doctor) {
    DoctorNode *node;
    DoctorNode **link = &list->head;
    if (!doctor) {
        return -1;
    }
    node = malloc(sizeof *node);
    if (!node) {
        return -1;
    }
    node->doctor = doctor;
    node->next = NULL;
    while (*link) {
        link = &(*link)->next;
    }
    *link = node;
    list->count++;
    return 0;
}

void *dlist_find(const DoctorList *list, DoctorMatch match, void *context) {
    const DoctorNode *n;
    for (n = list->head; n; n = n->next) {
        if (match(n->doctor, context)) {
            return n->doctor;
        }
    }
    return NULL;
}

void dlist_for_each(const DoctorList *list, DoctorVisit visit, void *context) {
    const DoctorNode *n;
    for (n = list->head; n; n = n->next) {
        visit(n->doctor, context);
    }
}

int dlist_count(const DoctorList *list) {
    return list->count;
}

void dlist_destroy(DoctorList *list) {
    DoctorNode *n = list->head;
    while (n) {
        DoctorNode *next = n->next;
        free(n);
        n = next;
    }
    list->head = NULL;
    list->count = 0;
}
