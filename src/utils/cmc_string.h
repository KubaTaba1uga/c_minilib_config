/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#ifndef C_MINILIB_CONFIG_CMC_STRING_H
#define C_MINILIB_CONFIG_CMC_STRING_H

#include <c_minilib_config.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void cmc_join_path_stack(char *out, size_t out_len,
                                       const char *dir_path,
                                       const char *config_name) {
  // Ensure null-termination on failure
  if (snprintf(out, out_len, "%s/%s", dir_path, config_name) >= (int)out_len) {
    out[out_len - 1] = '\0';
  }
}

static inline void cmc_join_str_stack(char *out, size_t out_len,
                                      const char *prefix, const char *suffix) {
  if (snprintf(out, out_len, "%s%s", prefix, suffix) >= (int)out_len) {
    out[out_len - 1] = '\0';
  }
}

static inline cme_error_t cmc_convert_str_to_int(char *str, uint32_t n,
                                                 int *output) {
  cme_error_t err;
  for (uint32_t i = 0; i < n; i++) {
    if (str[i] == 0) {
      break;
    }

    if (!isdigit(str[i])) {
      err = cme_errorf(EINVAL, "Unable to convert to integer `str=%s`\n", str);
      return err;
    }
  }

  char number[n + 1];
  memset(number, 0, n + 1);
  strncpy(number, str, n);

  *output = atoi(number);

  return NULL;
}

#endif // C_MINILIB_CONFIG_CMC_STRING_H
