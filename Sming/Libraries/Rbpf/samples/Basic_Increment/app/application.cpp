/*
 * Copyright (c) 2021 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       BPF tutorial 1 example
 *
 * @author      Koen Zandberg <koen.zandberg@inria.fr>
 *
 * @}
 */
#include <SmingCore.h>

#include <bpf.h>
#include <rbpf/containers.h>
#include <increment.h>
#include <int64/multiply.h>

namespace
{
/* Pre-allocated stack for the virtual machine */
static uint8_t stack[512]{};

using Container = FSTR::Array<uint8_t>;

/**
 * @brief Execute a container
 * @param container Container code to execute
 * @param ctx IN/OUT Passed to container
 * @retval int64_t Result returned from container
 */
template <typename Context> int64_t execute(const Container& container, Context& ctx)
{
	LOAD_FSTR_ARRAY(appBinary, container);

	/* Define the application */
	bpf_t bpf = {
		.application = appBinary,			   /* The increment.bin content */
		.application_len = container.length(), /* Length of the application */
		.stack = stack,						   /* Preallocated stack */
		.stack_size = sizeof(stack),		   /* And the length */
	};

	bpf_setup(&bpf);
	int64_t result{0};
	int err = bpf_execute_ctx(&bpf, &ctx, sizeof(ctx), &result);
	if(err != 0) {
		debug_e("Error! VM call failed with %d", err);
	}
	return result;
}

void test_increment()
{
	Serial.println(F("Calling 'increment()' in VM"));
	increment_context_t ctx{
		.value = 0,
	};
	auto res = execute(rBPF::Container::increment, ctx);
	Serial.print(_F("input "));
	Serial.print(ctx.value);
	Serial.print(_F(", result "));
	Serial.print(res);
	Serial.print(_F(", expected "));
	Serial.println(ctx.value + 1);
}

void test_multiply()
{
	Serial.println(F("Calling 'multiply()' in VM"));
	multiply_context_t ctx{
		.input1 = 120000005,
		.input2 = 120000023,
	};
	auto res = execute(rBPF::Container::int64_multiply, ctx);
	Serial.print(_F("input ("));
	Serial.print(ctx.input1);
	Serial.print(", ");
	Serial.print(ctx.input2);
	Serial.print(_F("), output "));
	Serial.print(ctx.output);
	Serial.print(_F(", expected "));
	Serial.print(int64_t(ctx.input1) * ctx.input2);
	Serial.print(_F(", result "));
	Serial.println(res);
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	/* Initialize the bpf subsystem */
	bpf_init();

	Serial.println("All up, running the Femto-Container application now");

	test_increment();
	test_multiply();
}
