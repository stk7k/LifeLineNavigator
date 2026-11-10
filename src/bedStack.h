#ifndef BEDSTACK_H
#define BEDSTACK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int *ids;
    int top;
    int capacity;
} BedStack;

int bedstack_init(BedStack *s, int capacity);
void bedstack_destroy(BedStack *s);
int bedstack_push(BedStack *s, int bed_id);
int bedstack_pop(BedStack *s, int *bed_id);
int bedstack_contains(const BedStack *s, int bed_id);
int bedstack_count(const BedStack *s);

#ifdef __cplusplus
}
#endif

#endif
