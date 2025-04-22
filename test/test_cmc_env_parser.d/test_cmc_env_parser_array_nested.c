#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unity.h>

#include "c_minilib_config.h"
#include "cmc_parse_interface/cmc_env_parser/cmc_env_parser.h"
#include "utils/cmc_error.h"
#include "utils/cmc_field.h"

#ifndef CONFIG_PATH
#define CONFIG_PATH "non_exsistent_path"
#endif

#ifndef ARRAY_CONFIG_PATH
#define ARRAY_CONFIG_PATH "non_exsistent_path"
#endif

#ifndef ARRAY_NESTED_CONFIG_PATH
#define ARRAY_NESTED_CONFIG_PATH "non_exsistent_path"
#endif

#ifndef KEA_CONFIG_PATH
#define KEA_CONFIG_PATH "non_exsistent_path"
#endif

static struct cmc_ConfigParseInterface parser;
static struct cmc_Config *config = NULL;
static cmc_error_t err = NULL;
