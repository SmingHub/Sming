/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef BPF_SHARED_H
#define BPF_SHARED_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __bpf_shared_ptr(type, name)    \
union {                 \
    type name;          \
    uint64_t :64;          \
} __attribute__((aligned(8)))

enum {
    /* Aux helper functions (stdlib) */
    BPF_FUNC_BPF_PRINTF = 0x01,
    BPF_FUNC_BPF_MEMCPY = 0x02,

    /* Key/value store functions */
    BPF_FUNC_BPF_STORE_LOCAL = 0x10,
    BPF_FUNC_BPF_STORE_GLOBAL = 0x11,
    BPF_FUNC_BPF_FETCH_LOCAL = 0x12,
    BPF_FUNC_BPF_FETCH_GLOBAL = 0x13,

    /* Time(r) functions */
    BPF_FUNC_BPF_NOW_MS = 0x20,

    /* Saul functions */
    BPF_FUNC_BPF_SAUL_REG_FIND_NTH = 0x30,
    BPF_FUNC_BPF_SAUL_REG_FIND_TYPE = 0x31,
    BPF_FUNC_BPF_SAUL_REG_READ = 0x32,

    /* (g)coap functions */
    BPF_FUNC_BPF_GCOAP_RESP_INIT = 0x40,
    BPF_FUNC_BPF_COAP_OPT_FINISH = 0x41,
    BPF_FUNC_BPF_COAP_ADD_FORMAT = 0x42,
    BPF_FUNC_BPF_COAP_GET_PDU = 0x43,

    BPF_FUNC_BPF_FMT_S16_DFP = 0x50,
    BPF_FUNC_BPF_FMT_U32_DEC = 0x51,

    /* ZTIMER */
    BPF_FUNC_BPF_ZTIMER_NOW = 0x60,
    BPF_FUNC_BPF_ZTIMER_PERIODIC_WAKEUP = 0x61,
};

/* Helper structs */
typedef struct {
    __bpf_shared_ptr(void*, pkt);      /**< Opaque pointer to the coap_pkt_t struct */
    __bpf_shared_ptr(uint8_t*, buf);   /**< Packet buffer */
    size_t buf_len; /**< Packet buffer length */
} bpf_coap_ctx_t;

#ifdef __cplusplus
}
#endif
#endif /* BPF_SHARED_H */
