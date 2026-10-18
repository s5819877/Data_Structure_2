#ifndef A8_KRUSKAL_H
#define A8_KRUSKAL_H

#include "A8_GraphCommon.h"
#include "A8_WListGraph.h"

int kruskal_mst(const ListGraph* g, Edge* out, MstStats* st);

#endif /* KRUSKAL_H */