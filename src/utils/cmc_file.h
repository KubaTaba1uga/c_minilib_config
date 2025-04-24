/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#ifndef C_MINILIB_CONFIG_CMC_FILE_H
#define C_MINILIB_CONFIG_CMC_FILE_H

#include <stdbool.h>
#include <unistd.h>

static inline bool cmc_does_file_exist(char *path) {
  return access(path, F_OK) == 0;
}

#endif // C_MINILIB_CONFIG_CMC_FILE_H
