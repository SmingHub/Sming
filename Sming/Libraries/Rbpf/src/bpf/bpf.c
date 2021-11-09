/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdint.h>
#include <stdbool.h>
#include "assert.h"
#include "bpf.h"
#include "bpf/store.h"

extern int bpf_run(bpf_t *bpf, const void *ctx, int64_t *result);

static bpf_hook_t *_hooks[BPF_HOOK_NUM] = { 0 };

static bool _continue(bpf_hook_t *hook, int64_t *res)
{
    switch(hook->policy) {
        case BPF_POLICY_CONTINUE:
            return true;
        case BPF_POLICY_SINGLE:
            return false;
        case BPF_POLICY_ABORT_ON_NEGATIVE:
            return (*res < 0) ? false : true;
        case BPF_POLICY_ABORT_ON_POSITIVE:
            return (*res > 0) ? false : true;
    }
    return true;
}

static int _execute(bpf_t *bpf, void *ctx, int64_t *result)
{
    assert(bpf->flags & BPF_FLAG_SETUP_DONE);
    return bpf_run(bpf, ctx, result);
}

int bpf_execute(bpf_t *bpf, void *ctx, size_t ctx_len, int64_t *result)
{
    (void)ctx;
    (void)ctx_len;
    bpf->arg_region.start = NULL;
    bpf->arg_region.len = 0;

    return _execute(bpf, ctx, result);
}

int bpf_execute_ctx(bpf_t *bpf, void *ctx, size_t ctx_len, int64_t *result)
{
    bpf->arg_region.start = ctx;
    bpf->arg_region.len = ctx_len;
    bpf->arg_region.flag = (BPF_MEM_REGION_READ | BPF_MEM_REGION_WRITE);

    return _execute(bpf, ctx, result);
}

void bpf_setup(bpf_t *bpf)
{
    bpf->stack_region.start = bpf->stack;
    bpf->stack_region.len = bpf->stack_size;
    bpf->stack_region.flag = (BPF_MEM_REGION_READ | BPF_MEM_REGION_WRITE);
    bpf->stack_region.next = &bpf->data_region;

    bpf->data_region.start = rbpf_data(bpf);
    bpf->data_region.len = rbpf_header(bpf)->data_len;
    bpf->data_region.flag = (BPF_MEM_REGION_READ | BPF_MEM_REGION_WRITE);
    bpf->data_region.next = &bpf->rodata_region;

    bpf->rodata_region.start = rbpf_rodata(bpf);
    bpf->rodata_region.len = rbpf_header(bpf)->rodata_len;
    bpf->rodata_region.flag = BPF_MEM_REGION_READ;
    bpf->rodata_region.next = &bpf->arg_region;

    bpf->arg_region.next = NULL;
    bpf->arg_region.start = NULL;
    bpf->arg_region.len = 0;

    bpf->flags |= BPF_FLAG_SETUP_DONE;
}

void bpf_add_region(bpf_t *bpf, bpf_mem_region_t *region,
                    void *start, size_t len, uint8_t flags)
{
    region->next = bpf->arg_region.next;
    region->start = start;
    region->len = len;
    region->flag = flags;
    bpf->arg_region.next = region;
}

static void _register(bpf_hook_t **install_hook, bpf_hook_t *new)
{
    new->next = *install_hook;
    *install_hook = new;
}

void bpf_init(void)
{
    bpf_store_init();
}

int bpf_hook_install(bpf_hook_t *hook, bpf_hook_trigger_t trigger) {
    assert(trigger < BPF_HOOK_NUM);
    _register(&_hooks[trigger], hook);
    return 0;
}

int bpf_hook_execute(bpf_hook_trigger_t trigger, void *ctx, size_t ctx_size, int64_t *script_res)
{
    assert(trigger < BPF_HOOK_NUM);

    int res = BPF_OK;

    for (bpf_hook_t *h = _hooks[trigger]; h; h = h->next) {
        res = bpf_execute_ctx(h->application, ctx, ctx_size, script_res);
        h->executions++;
        if ((res == BPF_OK) && !_continue(h, script_res)) {
            break;
        }
    }
    return res;
}
