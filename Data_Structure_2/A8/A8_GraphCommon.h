#ifndef A8_GRAPHCOMMON_H
#define A8_GRAPHCOMMON_H

#include <stddef.h>

#define N_VERTICES   50 
#define SPARSE_EDGES 100 
#define DENSE_EDGES  600 
#define WEIGHT_MAX   100 

typedef struct { int u, v, w; } Edge;

#define MEM_MAX_ITEMS 8
typedef struct { const char* name; size_t bytes; } MemItem;
typedef struct {
    MemItem item[MEM_MAX_ITEMS];
    int     count;
    size_t  total;
} MemReport;

static inline void mem_reset(MemReport* m) { m->count = 0; m->total = 0; }
static inline void mem_add(MemReport* m, const char* name, size_t bytes)
{
    if (m->count < MEM_MAX_ITEMS) {
        m->item[m->count].name = name;
        m->item[m->count].bytes = bytes;
        m->count++;
    }
    m->total += bytes;
}

typedef struct {
    int       edge_count;
    long      cost;
    MemReport aux;

    long ops_collect; 
    long ops_sort;
    long ops_examined;

    long ops_select;
    long ops_cells;
} MstStats;

#endif 