/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#ifndef C_MINILIB_CONFIG_CMC_ENV_PARSER_H
#define C_MINILIB_CONFIG_CMC_ENV_PARSER_H

#include "cmc_parse_interface/cmc_parse_interface.h"
#include <c_minilib_config.h>

cme_error_t cmc_env_parser_init(struct cmc_ConfigParseInterface *parser);

#endif // C_MINILIB_CONFIG_CMC_ENV_PARSER_H
