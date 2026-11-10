#ifndef BILLING_H
#define BILLING_H

#ifdef __cplusplus
extern "C" {
#endif

#define CHARGE_DESC_LEN 64

typedef struct {
    char description[CHARGE_DESC_LEN];
    long long amount_paise;
} Charge;

typedef struct {
    Charge *items;
    int count;
    int capacity;
} ChargeList;

int charges_init(ChargeList *list);
void charges_destroy(ChargeList *list);
int charges_add(ChargeList *list, const char *description, long long amount_paise);
long long charges_total(const ChargeList *list);
int charges_count(const ChargeList *list);
const Charge *charges_get(const ChargeList *list, int index);

#ifdef __cplusplus
}
#endif

#endif
