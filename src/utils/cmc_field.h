/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#ifndef C_MINILIB_CONFIG_CMC_FIELD_H
#define C_MINILIB_CONFIG_CMC_FIELD_H

#include "c_minilib_config.h"
#include "utils/cmc_tree.h"

cme_error_t cmc_field_add_value_str(struct cmc_ConfigField *field,
                                    const char *value);

cme_error_t cmc_field_add_value_int(struct cmc_ConfigField *field,
                                    const int32_t value);

void cmc_field_destroy(struct cmc_ConfigField **field);

#endif // C_MINILIB_CONFIG_CMC_FIELD_H
