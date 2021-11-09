/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef BPF_BPFAPI_COAP_H
#define BPF_BPFAPI_COAP_H

#include <stdint.h>
#include "bpf/shared.h"
#include "net/coap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *pkt;      /**< Opaque pointer to the coap_pkt_t struct */
    uint8_t *buf;   /**< Packet buffer */
    size_t buf_len; /**< Packet buffer length */
} bpf_coap_ctx_t;

#ifdef __cplusplus
}
#endif
#endif /* BPF_BPFAPI_COAP_H */
