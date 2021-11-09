/*
 * Copyright (C) 2021 Inria
 * Copyright (C) 2021 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bpf.h"
#include "bpf/instruction.h"
#include "bpf/call.h"

static bool _bpf_check_call(uint32_t num)
{
    switch(num) {
        /* These calls are expected to be supported */
        case BPF_FUNC_BPF_PRINTF:
        case BPF_FUNC_BPF_STORE_LOCAL:
        case BPF_FUNC_BPF_STORE_GLOBAL:
        case BPF_FUNC_BPF_FETCH_LOCAL:
        case BPF_FUNC_BPF_FETCH_GLOBAL:
        case BPF_FUNC_BPF_NOW_MS:
        case BPF_FUNC_BPF_SAUL_REG_FIND_NTH:
        case BPF_FUNC_BPF_SAUL_REG_FIND_TYPE:
        case BPF_FUNC_BPF_SAUL_REG_READ:
#ifdef MODULE_GCOAP
        case BPF_FUNC_BPF_GCOAP_RESP_INIT:
        case BPF_FUNC_BPF_COAP_OPT_FINISH:
#endif
            return true;
        default:
            return false;
    }
}


int bpf_verify_preflight(bpf_t *bpf)
{
    const bpf_instruction_t *application = rbpf_text(bpf);
    size_t length = rbpf_text_len(bpf);
    if (bpf->flags & BPF_FLAG_PREFLIGHT_DONE) {
        return BPF_OK;
    }

    if (length & 0x7) {
        return BPF_ILLEGAL_LEN;
    }


    for (const bpf_instruction_t *i = application;
            i < (bpf_instruction_t*)((uint8_t*)application + length); i++) {
        /* Check if register values are valid */
        if (i->dst >= 11 || i->src >= 11) {
            return BPF_ILLEGAL_REGISTER;
        }

        /* Double length instruction */
        if (i->opcode == 0x18) {
            i++;
            continue;
        }

        /* Only instruction-specific checks here */
        if ((i->opcode & BPF_INSTRUCTION_CLS_MASK) == BPF_INSTRUCTION_CLS_BRANCH) {
            intptr_t target = (intptr_t)(i + i->offset);
            /* Check if the jump target is within bounds. The address is
             * incremented after the jump by the regular PC increase */
            if ((target >= (intptr_t)((uint8_t*)application + length))
                || (target < (intptr_t)application)) {
                return BPF_ILLEGAL_JUMP;
            }
        }

        if (i->opcode == (BPF_INSTRUCTION_BRANCH_CALL | BPF_INSTRUCTION_CLS_BRANCH)) {
            if (!_bpf_check_call(i->immediate)) {
                return BPF_ILLEGAL_CALL;
            }
        }
    }

    size_t num_instructions = length/sizeof(bpf_instruction_t);

    /* Check if the last instruction is a return instruction */
    if (application[num_instructions - 1].opcode != 0x95 && !(bpf->flags & BPF_CONFIG_NO_RETURN)) {
        return BPF_NO_RETURN;
    }
    bpf->flags |= BPF_FLAG_PREFLIGHT_DONE;
    return BPF_OK;
}
