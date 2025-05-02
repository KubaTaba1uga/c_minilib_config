/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#ifndef C_MINILIB_CONFIG_CMC_SETTINGS_H
#define C_MINILIB_CONFIG_CMC_SETTINGS_H

#include "c_minilib_config.h"

cme_error_t cmc_settings_create(const uint32_t paths_length,
                                const char *supported_paths[paths_length],
                                const char *name, const void *log_func,
                                struct cmc_ConfigSettings **settings);

void cmc_settings_destroy(struct cmc_ConfigSettings **settings);

#endif // C_MINILIB_CONFIG_CMC_SETTINGS_H
