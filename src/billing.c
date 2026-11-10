#include <stdio.h>
#include <stdlib.h>
#include "billing.h"

#define INITIAL_CAPACITY 4

int charges_init(ChargeList *list) {
    list->items = malloc(INITIAL_CAPACITY * sizeof *list->items);
    list->count = 0;
    list->capacity = list->items ? INITIAL_CAPACITY : 0;
    return list->items ? 0 : -1;
}

void charges_destroy(ChargeList *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static int grow(ChargeList *list) {
    int new_capacity = list->capacity ? list->capacity * 2 : INITIAL_CAPACITY;
    Charge *grown = realloc(list->items, (size_t)new_capacity * sizeof *grown);
    if (!grown) {
        return -1;
    }
    list->items = grown;
    list->capacity = new_capacity;
    return 0;
}

int charges_add(ChargeList *list, const char *description, long long amount_paise) {
    Charge *c;
    if (!description || amount_paise < 0) {
        return -1;
    }
    if (list->count == list->capacity && grow(list) != 0) {
        return -1;
    }
    c = &list->items[list->count];
    snprintf(c->description, CHARGE_DESC_LEN, "%s", description);
    c->amount_paise = amount_paise;
    list->count++;
    return 0;
}

long long charges_total(const ChargeList *list) {
    long long total = 0;
    int i;
    for (i = 0; i < list->count; i++) {
        total += list->items[i].amount_paise;
    }
    return total;
}

int charges_count(const ChargeList *list) {
    return list->count;
}

const Charge *charges_get(const ChargeList *list, int index) {
    if (index < 0 || index >= list->count) {
        return NULL;
    }
    return &list->items[index];
}
