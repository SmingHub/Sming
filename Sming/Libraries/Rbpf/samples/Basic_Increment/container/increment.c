#include "increment.h"

// Modified context is returned to caller
int increment(increment_context_t* context)
{
	return context->value + 1;
}
