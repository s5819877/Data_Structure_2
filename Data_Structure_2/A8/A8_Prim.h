#ifndef A8_PRIM_H
#define A8_PRIM_H

#include "A8_GraphCommon.h"
#include "A8_WMatrixGraph.h"

int prim_mst(const MatrixGraph* g, int start, Edge* out, MstStats* st);

#endif