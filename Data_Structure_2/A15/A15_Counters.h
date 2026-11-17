#ifndef A15_COUNTERS_H
#define A15_COUNTERS_H
#include <stddef.h>

#define FRAME_BYTES_MODEL 64      

typedef struct {
    long long cmp;          
    long long mov;          
    int       max_depth;    
    long long rot;          
    size_t    bytes_now;    
    size_t    bytes_peak;   
} Counters;

static inline void ctr_init(Counters* c) { c->cmp = c->mov = c->rot = 0; c->max_depth = 0; c->bytes_now = c->bytes_peak = 0; }
static inline unsigned long long stack_model_bytes(const Counters* c) { return (unsigned long long)(c->max_depth + 1) * FRAME_BYTES_MODEL; }

#endif