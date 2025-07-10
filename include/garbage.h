#ifndef GARB_H
#define GARB_H

#include "types.h"

struct Collector 
{
	Expr **exprs;
	int n_exprs;
};

void collect(struct Collector *gc, PairTable *pt);

#endif
