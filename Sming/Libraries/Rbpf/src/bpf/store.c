/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdint.h>
#include <stddef.h>
#include "btree.h"
#include "bpf.h"
#include "bpf/store.h"
#include "memarray.h"

static btree_t _global;

/* Singleton mem array */
static memarray_t _array;
static bpf_store_keyval_t _vals[CONFIG_BPF_STORE_NUM_VALUES];

void bpf_store_init(void)
{
    memarray_init(&_array, _vals, sizeof(bpf_store_keyval_t),
                  CONFIG_BPF_STORE_NUM_VALUES);
}

static int _alloc_value(btree_t *tree, uint32_t key, uint32_t value)
{
    bpf_store_keyval_t *keyval = memarray_alloc(&_array);
    bpf_store_set_value(keyval, value);
    if (!keyval) {
        return -1;
    }
    btree_insert(tree, &keyval->node, key);
    return 0;
}

static int _fetch_value(btree_t *tree, uint32_t key, uint32_t *value)
{
    bpf_store_keyval_t *keyval = (bpf_store_keyval_t*)btree_find_key(tree, key);
    if (!keyval) {
        *value = 0;
        return _alloc_value(tree, key, *value);
    }
    *value = bpf_store_get_value(keyval);
    return 0;
}

static int _store_value(btree_t *tree, uint32_t key, uint32_t value)
{
    bpf_store_keyval_t *keyval = (bpf_store_keyval_t*)btree_find_key(tree, key);
    if (!keyval) {
        return _alloc_value(tree, key, value);
    }
    bpf_store_set_value(keyval, value);
    return 0;
}

int bpf_store_update_global(uint32_t key, uint32_t value)
{
    return _store_value(&_global, key, value);
}

int bpf_store_update_local(bpf_t *bpf, uint32_t key, uint32_t value)
{
    return _store_value(&bpf->btree, key, value);
}

int bpf_store_fetch_global(uint32_t key, uint32_t *value)
{
/* fetch value from the global store if it exists, otherwise add it and return a
 * default value */
    return _fetch_value(&_global, key, value);
}

int bpf_store_fetch_local(bpf_t *bpf, uint32_t key, uint32_t *value)
{
/* fetch value from the bpf local store if it exists, otherwise add it and return a
 * default value */
    return _fetch_value(&bpf->btree, key, value);
}

void bpf_store_iter_global(btree_cb_t cb, void *ctx)
{
    btree_traverse(&_global, cb, ctx);
}
