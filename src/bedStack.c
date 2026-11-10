#include <stdlib.h>
#include "bedStack.h"

int bedstack_init(BedStack *s, int capacity) {
    if (capacity <= 0) {
        return -1;
    }
    s->ids = malloc((size_t)capacity * sizeof *s->ids);
    if (!s->ids) {
        return -1;
    }
    s->top = 0;
    s->capacity = capacity;
    return 0;
}

void bedstack_destroy(BedStack *s) {
    free(s->ids);
    s->ids = NULL;
    s->top = 0;
    s->capacity = 0;
}

int bedstack_contains(const BedStack *s, int bed_id) {
    int i;
    for (i = 0; i < s->top; i++) {
        if (s->ids[i] == bed_id) {
            return 1;
        }
    }
    return 0;
}

int bedstack_push(BedStack *s, int bed_id) {
    if (s->top == s->capacity || bedstack_contains(s, bed_id)) {
        return -1;
    }
    s->ids[s->top++] = bed_id;
    return 0;
}

int bedstack_pop(BedStack *s, int *bed_id) {
    if (s->top == 0) {
        return -1;
    }
    *bed_id = s->ids[--s->top];
    return 0;
}

int bedstack_count(const BedStack *s) {
    return s->top;
}
