/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_bpf_instructions eBPF instructions
 * @ingroup     sys_bpf
 * @brief       Definitions for eBPF instructions
 *
 * @{
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef BPF_INSTRUCTION_H
#define BPF_INSTRUCTION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BPF_INSTRUCTION_CLS_MASK        0x07

#define BPF_INSTRUCTION_CLS_LD          0x00
#define BPF_INSTRUCTION_CLS_LDX         0x01
#define BPF_INSTRUCTION_CLS_ST          0x02
#define BPF_INSTRUCTION_CLS_STX         0x03
#define BPF_INSTRUCTION_CLS_ALU32       0x04
#define BPF_INSTRUCTION_CLS_BRANCH      0x05
#define BPF_INSTRUCTION_CLS_ALU64       0x07

#define BPF_INSTRUCTION_MEM_CLS_MASK    0x07
#define BPF_INSTRUCTION_MEM_SZ_MASK     0x18
#define BPF_INSTRUCTION_MEM_MDE_MASK    0xE0

#define BPF_INSTRUCTION_ALU_CLS_MASK    0x07
#define BPF_INSTRUCTION_ALU_S_MASK      0x08
#define BPF_INSTRUCTION_ALU_OP_MASK     0xf0

#define BPF_INSTRUCTION_LDX_LDX         0x60

#define BPF_INSTRUCTION_STX_ST          0x60

#define BPF_INSTRUCTION_STX_STX         0x60


#define BPF_INSTRUCTION_ALU_ADD         0x00
#define BPF_INSTRUCTION_ALU_SUB         0x10
#define BPF_INSTRUCTION_ALU_MUL         0x20
#define BPF_INSTRUCTION_ALU_DIV         0x30
#define BPF_INSTRUCTION_ALU_OR          0x40
#define BPF_INSTRUCTION_ALU_AND         0x50
#define BPF_INSTRUCTION_ALU_LSH         0x60
#define BPF_INSTRUCTION_ALU_RSH         0x70
#define BPF_INSTRUCTION_ALU_NEG         0x80
#define BPF_INSTRUCTION_ALU_MOD         0x90
#define BPF_INSTRUCTION_ALU_XOR         0xA0
#define BPF_INSTRUCTION_ALU_MOV         0xB0
#define BPF_INSTRUCTION_ALU_ARSH        0xC0

#define BPF_INSTRUCTION_BRANCH_JA       0x00
#define BPF_INSTRUCTION_BRANCH_JEQ      0x10
#define BPF_INSTRUCTION_BRANCH_JGT      0x20
#define BPF_INSTRUCTION_BRANCH_JGE      0x30
#define BPF_INSTRUCTION_BRANCH_JLT      0xa0
#define BPF_INSTRUCTION_BRANCH_JLE      0xb0
#define BPF_INSTRUCTION_BRANCH_JSET     0x40
#define BPF_INSTRUCTION_BRANCH_JNE      0x50
#define BPF_INSTRUCTION_BRANCH_JSGT     0x60
#define BPF_INSTRUCTION_BRANCH_JSGE     0x70
#define BPF_INSTRUCTION_BRANCH_JSLT     0xc0
#define BPF_INSTRUCTION_BRANCH_JSLE     0xd0
#define BPF_INSTRUCTION_BRANCH_CALL     0x80
#define BPF_INSTRUCTION_BRANCH_EXIT     0x90

#define BPF_INSTRUCTION_ALU_BYTESWAP    0xd0

/**
 * @brief eBPF instruction format
 *
 * Always in host byte order
 */
typedef struct __attribute__((packed)) {
    uint8_t opcode;
    unsigned dst:4;
    unsigned src:4;
    int16_t offset;
    int32_t immediate;
} bpf_instruction_t;

#ifdef __cplusplus
}
#endif
#endif /* BPF_INSTRUCTION_H */
/** @} */

