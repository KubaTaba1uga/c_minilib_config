/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#ifndef C_MINILIB_CONFIG_CMC_PARSE_INTERFACE_H
#define C_MINILIB_CONFIG_CMC_PARSE_INTERFACE_H

#include <c_minilib_config.h>
#include <stdbool.h>
#include <stddef.h>

typedef void *cmc_ConfigParserData;

enum cmc_ConfigParseFormat {
  cmc_ConfigParseFormat_NONE,
  cmc_ConfigParseFormat_ENV,
  cmc_ConfigParseFormat_JSON,
  // Add new formats here
  cmc_ConfigParseFormat_MAX,
};

struct cmc_ConfigParseInterface {
  const char *id;
  cmc_ConfigParserData data;
  cmc_error_t (*create)(cmc_ConfigParserData *);
  // Decide if parser can parse a file based on it's path
  cmc_error_t (*is_format)(const size_t n, const char path[n], bool *result);
  // Parse a file based on it's path
  cmc_error_t (*parse)(const size_t n, const char path[n],
                       const cmc_ConfigParserData data,
                       struct cmc_Config *config);
  // Destroy parser instance
  void (*destroy)(cmc_ConfigParserData *);
};

#endif // C_MINILIB_CONFIG_PARSE_INTERFACE_H
