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
#include <rbpf/container/increment.h>

namespace
{
/* Pre-allocated stack for the virtual machine */
static uint8_t stack[512] = {0};

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	/* Initialize the bpf subsystem */
	bpf_init();

	Serial.println("All up, running the Femto-Container application now");

	LOAD_FSTR_ARRAY(appBinary, rBPF::Container::increment);

	/* Define the application */
	bpf_t bpf = {
		.application = appBinary,								/* The increment.bin content */
		.application_len = rBPF::Container::increment.length(), /* Length of the application */
		.stack = stack,											/* Preallocated stack */
		.stack_size = sizeof(stack),							/* And the length */
	};

	/* Context value to pass to the VM */
	uint64_t ctx = 0;
	int64_t result = 0;

	bpf_setup(&bpf);
	int res = bpf_execute_ctx(&bpf, &ctx, sizeof(ctx), &result);

	Serial.print(_F("Input to the VM: "));
	Serial.println(ctx);
	Serial.print(_F("Return code (expected 0): "));
	Serial.println(res);
	Serial.print(_F("Result of the VM: "));
	Serial.println(result);
}
