/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef BPF_CALL_H
#define BPF_CALL_H

#include <stdint.h>
#include "bpf/shared.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*bpf_call_t)(bpf_t *bpf, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);

uint32_t bpf_vm_printf(bpf_t *bpf, uint32_t fmt, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4);
uint32_t bpf_vm_store_local(bpf_t *bpf, uint32_t fmt, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4);
uint32_t bpf_vm_store_global(bpf_t *bpf, uint32_t fmt, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4);
uint32_t bpf_vm_fetch_local(bpf_t *bpf, uint32_t fmt, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4);
uint32_t bpf_vm_fetch_global(bpf_t *bpf, uint32_t fmt, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4);
uint32_t bpf_vm_memcpy(bpf_t *bpf, uint32_t dest_p, uint32_t src_p, uint32_t size, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_now_ms(bpf_t *bpf, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_saul_reg_find_nth(bpf_t *bpf, uint32_t nth, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_saul_reg_find_type(bpf_t *bpf, uint32_t type, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_saul_reg_read(bpf_t *bpf, uint32_t dev_p, uint32_t data_p, uint32_t a3, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_gcoap_resp_init(bpf_t *bpf, uint32_t coap_ctx_p, uint32_t resp_code_u, uint32_t a3, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_coap_opt_finish(bpf_t *bpf, uint32_t coap_ctx_p, uint32_t flags_u, uint32_t a3, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_fmt_s16_dfp(bpf_t *bpf, uint32_t out_p, uint32_t val, uint32_t fp_digits, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_fmt_u32_dec(bpf_t *bpf, uint32_t out_p, uint32_t val, uint32_t a3, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_coap_add_format(bpf_t *bpf, uint32_t coap_ctx_p, uint32_t format, uint32_t a3, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_coap_get_pdu(bpf_t *bpf, uint32_t coap_ctx_p, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);


uint32_t bpf_vm_ztimer_now(bpf_t *bpf, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
uint32_t bpf_vm_ztimer_periodic_wakeup(bpf_t *bpf, uint32_t last_wakeup_p, uint32_t period, uint32_t a3, uint32_t a4, uint32_t a5);
#ifdef __cplusplus
}
#endif
#endif /* BPF_CALL_H */

