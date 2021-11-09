/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_bpf_reg Virtual machine registration
 * @ingroup     sys
 * @brief       BPF script registry
 *
 *
 * @{
 *
 * @file
 * @brief       BPF registry
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef BPF_REG_H
#define BPF_REG_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
} bpf_reg_t

int bpf_reg_add(void);

int bpf_reg_unlink(void);

bpf_t *bpf_reg_find(void);

#ifdef __cplusplus
}
#endif
#endif /* BPF_REG_H */
/** @} */
