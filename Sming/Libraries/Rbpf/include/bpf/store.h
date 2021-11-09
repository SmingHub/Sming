/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_bpf_store BPF key-value store
 * @ingroup     sys_bpf
 * @brief       API for the eBPF key-value store
 *
 *
 * @{
 *
 * @file
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef BPF_STORE_H
#define BPF_STORE_H

#include <stdint.h>
#include <stdlib.h>
#include "btree.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_BPF_STORE_NUM_VALUES
#define CONFIG_BPF_STORE_NUM_VALUES     (16U)
#endif /* CONFIG_BPF_STORE_NUM_VALUES */

/**
 * @brief eBPF key-value object
 */
typedef struct {
    btree_node_t node; /**< Binary search tree node */
    uint32_t value;     /**< Value */
} bpf_store_keyval_t;

static inline uint32_t bpf_store_get_key(bpf_store_keyval_t *keyval)
{
    return btree_node_key(&keyval->node);
}

static inline uint32_t bpf_store_get_value(bpf_store_keyval_t *keyval)
{
    return keyval->value;
}

static inline void bpf_store_set_value(bpf_store_keyval_t *keyval, uint32_t val)
{
    keyval->value = val;
}

void bpf_store_init(void);
int bpf_store_update_global(uint32_t key, uint32_t value);
int bpf_store_update_local(bpf_t *bpf, uint32_t key, uint32_t value);
int bpf_store_fetch_global(uint32_t key, uint32_t *value);
int bpf_store_fetch_local(bpf_t *bpf, uint32_t key, uint32_t *value);
void bpf_store_iter_global(btree_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif
#endif /* BPF_STORE_H */
/** @} */
