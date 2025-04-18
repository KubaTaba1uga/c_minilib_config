// File: test/test_env_parser.d/test_env_parser_config.c

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
#define CONFIG_PATH "akjskajsdhkahjd"
#endif

static struct cmc_ConfigParseInterface parser;
static struct cmc_ConfigField *field_str;
static struct cmc_ConfigField *field_int;
static struct cmc_Config *config;
static cmc_error_t err;

void setUp(void) {
  err = cmc_env_parser_init(&parser);
  TEST_ASSERT_NULL(err);
  field_str = NULL;
  field_int = NULL;
  config = NULL;
}

void tearDown(void) {
  cmc_field_destroy(&field_str);
  cmc_field_destroy(&field_int);
  cmc_config_destroy(&config);
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

void test_parse_valid_env_file(void) {
  // Define config settings with correct CONFIG_PATH and default name "config"
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "config",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  // Register string field (name must match lowercased env key)
  err = cmc_field_create("cmc_str_config", cmc_ConfigFieldTypeEnum_STRING,
                         "default_value", true, &field_str);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(field_str, config);
  TEST_ASSERT_NULL(err);

  // Register int field (same rule)
  int default_val = 0;
  err = cmc_field_create("cmc_int_config", cmc_ConfigFieldTypeEnum_INT,
                         &default_val, true, &field_int);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(field_int, config);
  TEST_ASSERT_NULL(err);

  // Parse the .env file
  err = parser.parse(strlen(CONFIG_PATH), CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  // Assert values parsed correctly
  TEST_ASSERT_NOT_NULL(field_str->value);
  TEST_ASSERT_NOT_NULL(field_int->value);
  TEST_ASSERT_EQUAL_STRING("whatever", (char *)field_str->value);
  TEST_ASSERT_EQUAL_INT(1, *(int *)field_int->value);
}
