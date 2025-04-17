// File: test/test_env_parser.d/test_env_parser_config.c

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unity.h>

#include "cmc_parse_interface/cmc_env_parser/cmc_env_parser.h"

#ifndef CONFIG_PATH
#define CONFIG_PATH "akjskajsdhkahjd"
#endif

static struct cmc_ConfigParseInterface parser;
static cmc_error_t err;

void setUp(void) {
  err = cmc_env_parser_init(&parser);
  TEST_ASSERT_NULL(err);
}

void tearDown(void) {
  // nothing to clean up
}

void test_is_format_for_existing_config_env(void) {
  bool result = false;
  err = parser.is_format(strlen(CONFIG_PATH), CONFIG_PATH, &result);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_TRUE(result);
}

void test_is_format_for_nonexistent_file(void) {
  const char *base = "does_not_exist";
  bool result = true;
  err = parser.is_format(strlen(base), base, &result);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_FALSE(result);
}
