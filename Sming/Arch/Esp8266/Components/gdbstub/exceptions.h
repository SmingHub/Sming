/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * exceptions.h
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <xtensa/corebits.h>
#include "gdb/signals.h"

/*
 * Trapped exceptions and the corresponding GDB signal
 * Ordered from 0 to last used exception
 *
 * EXCCAUSE_*, signal, desc
 *
 * Only those exceptions where `signal` is non-zero are trapped
 */
#define SYSTEM_EXCEPTION_MAP(XX)                                                                                       \
	XX(ILLEGAL, GDB_SIGNAL_ILL, "Illegal Instruction")                                                                 \
	XX(SYSCALL, GDB_SIGNAL_USR2, "System Call")                                                                        \
	XX(INSTR_ERROR, GDB_SIGNAL_SEGV, "Instruction Fetch Error")                                                        \
	XX(LOAD_STORE_ERROR, GDB_SIGNAL_SEGV, "Load Store Error")                                                          \
	XX(LEVEL1_INTERRUPT, 0, "Level 1 Interrupt")                                                                       \
	XX(ALLOCA, 0, "MOVSP inst")                                                                                        \
	XX(DIVIDE_BY_ZERO, GDB_SIGNAL_FPE, "Integer Divide by Zero")                                                       \
	XX(SPECULATION, 0, "")                                                                                             \
	XX(PRIVILEGED, GDB_SIGNAL_ABRT, "Privileged Instruction")                                                          \
	XX(UNALIGNED, GDB_SIGNAL_EMT, "Unaligned Load/Store")                                                              \
	XX(RSVD_10, 0, "")                                                                                                 \
	XX(RSVD_11, 0, "")                                                                                                 \
	XX(INSTR_DATA_ERROR, GDB_SIGNAL_EMT, "PIF Data Error on Instruction Fetch")                                        \
	XX(LOAD_STORE_DATA_ERROR, GDB_SIGNAL_EMT, "PIF Data Error on Load/Store")                                          \
	XX(INSTR_ADDR_ERROR, GDB_SIGNAL_EMT, "PIF Address Error on Instruction Fetch")                                     \
	XX(LOAD_STORE_ADDR_ERROR, GDB_SIGNAL_EMT, "PIF Address Error on Load/Store")                                       \
	XX(ITLB_MISS, 0, "ITLB Miss")                                                                                      \
	XX(ITLB_MULTIHIT, 0, "ITLB Multihit")                                                                              \
	XX(INSTR_RING, 0, "Ring Privilege Violation on Instruction Fetch")                                                 \
	XX(RSVD_19, 0, "")                                                                                                 \
	XX(INSTR_PROHIBITED, GDB_SIGNAL_SEGV, "Cache Attribute does not allow Instruction Fetch")                          \
	XX(RSVD_21, 0, "")                                                                                                 \
	XX(RSVD_22, 0, "")                                                                                                 \
	XX(RSVD_23, 0, "")                                                                                                 \
	XX(DTLB_MISS, 0, "DTLB Miss")                                                                                      \
	XX(DTLB_MULTIHIT, 0, "TBLD Multihit")                                                                              \
	XX(LOAD_STORE_RING, 0, "Ring Privilege Violation on Load/Store")                                                   \
	XX(RSVD_27, 0, "")                                                                                                 \
	XX(LOAD_PROHIBITED, GDB_SIGNAL_SEGV, "Cache Attribute does not allow Load")                                        \
	XX(STORE_PROHIBITED, GDB_SIGNAL_SEGV, "Cache Attribute does not allow Store")

#define EXCCAUSE_MAX EXCCAUSE_STORE_PROHIBITED
