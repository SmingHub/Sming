#include "multiply.h"

int multiply(multiply_context_t* context)
{
	context->output = int64_t(context->input1) * context->input2;
	return 0;
}
