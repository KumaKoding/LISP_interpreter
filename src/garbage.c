#include "garbage.h"

void collect(struct Collector *gc, PairTable *pt)
{
	for(int i = 0; i < gc->n_exprs; i++)
	{
		if(gc->exprs[i]->ref > 0)
		{
		}
	}
}

