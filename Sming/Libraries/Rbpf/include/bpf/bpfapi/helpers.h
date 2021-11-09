/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef BPF_BPFAPI_HELPERS_H
#define BPF_BPFAPI_HELPERS_H

#include <stdint.h>
#include "bpf/shared.h"
#include "phydat.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef signed ssize_t;

/**
 * Opaque dummy type saul registration
 */
typedef void bpf_saul_reg_t;

static void *(*bpf_printf)(const char *fmt, ...) = (void *) BPF_FUNC_BPF_PRINTF;

static int (*bpf_store_global)(uint32_t key, uint32_t value) = (void *) BPF_FUNC_BPF_STORE_GLOBAL;
static int (*bpf_store_local)(uint32_t key, uint32_t value) = (void *) BPF_FUNC_BPF_STORE_LOCAL;
static int (*bpf_fetch_global)(uint32_t key, uint32_t *value) = (void *) BPF_FUNC_BPF_FETCH_GLOBAL;
static int (*bpf_fetch_local)(uint32_t key, uint32_t *value) = (void *) BPF_FUNC_BPF_FETCH_LOCAL;
static uint32_t (*bpf_now_ms)(void) = (void *) BPF_FUNC_BPF_NOW_MS;

/* STDLIB */
static void *(*bpf_memcpy)(void *dest, const void *src, size_t n) = (void *) BPF_FUNC_BPF_MEMCPY;

/* SAUL calls */
static bpf_saul_reg_t *(*bpf_saul_reg_find_nth)(int pos) = (void *) BPF_FUNC_BPF_SAUL_REG_FIND_NTH;
static bpf_saul_reg_t *(*bpf_saul_reg_find_type)(uint8_t type) = (void *) BPF_FUNC_BPF_SAUL_REG_FIND_TYPE;
static int (*bpf_saul_reg_read)(bpf_saul_reg_t *dev, phydat_t *data) = (void *) BPF_FUNC_BPF_SAUL_REG_READ;

/* CoAP calls */
static void (*bpf_gcoap_resp_init)(bpf_coap_ctx_t *ctx, unsigned resp_code) = (void *) BPF_FUNC_BPF_GCOAP_RESP_INIT;
static ssize_t (*bpf_coap_opt_finish)(bpf_coap_ctx_t *ctx, unsigned opt) = (void *) BPF_FUNC_BPF_COAP_OPT_FINISH;
static void (*bpf_coap_add_format)(bpf_coap_ctx_t *ctx, uint32_t format) = (void *) BPF_FUNC_BPF_COAP_ADD_FORMAT;
static uint8_t *(*bpf_coap_get_pdu)(bpf_coap_ctx_t *ctx) = (void *) BPF_FUNC_BPF_COAP_GET_PDU;

/* FMT calls */
static size_t (*bpf_fmt_s16_dfp)(char *out, int16_t val, int fp_digits) = (void *) BPF_FUNC_BPF_FMT_S16_DFP;
static size_t (*bpf_fmt_u32_dec)(char *out, uint32_t val) = (void *) BPF_FUNC_BPF_FMT_U32_DEC;

/* ZTIMER calls */
static uint32_t (*bpf_ztimer_now)(void) = (void *) BPF_FUNC_BPF_ZTIMER_NOW;
static void (*bpf_ztimer_periodic_wakeup)(uint32_t *last_wakeup, uint32_t period) = (void *) BPF_FUNC_BPF_ZTIMER_PERIODIC_WAKEUP;

#ifdef __cplusplus

}
#endif
#endif /* BPF_APPLICATION_CALL_H */
