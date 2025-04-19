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
static struct cmc_ConfigField *field_str = NULL;
static struct cmc_ConfigField *field_int = NULL;
static struct cmc_ConfigField *field_empty = NULL;
static struct cmc_Config *config = NULL;
static cmc_error_t err = NULL;

void setUp(void) {
  err = cmc_env_parser_init(&parser);
  TEST_ASSERT_NULL(err);
  config = NULL;
}

void tearDown(void) {
  cmc_config_destroy(&config);
  cmc_error_destroy(&err);
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
  // Create config with one supported path and name
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "config",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  // Create fields
  err = cmc_field_create("cmc_str_config", cmc_ConfigFieldTypeEnum_STRING,
                         "default_value", true, &field_str);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(field_str, config);
  TEST_ASSERT_NULL(err);

  static int default_val = 0;
  err = cmc_field_create("cmc_int_config", cmc_ConfigFieldTypeEnum_INT,
                         &default_val, true, &field_int);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(field_int, config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("cmc_empty_config", cmc_ConfigFieldTypeEnum_STRING,
                         "default_value", true, &field_empty);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(field_empty, config);
  TEST_ASSERT_NULL(err);

  // Parse .env file
  err = parser.parse(strlen(CONFIG_PATH), CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  // Validate parsed values
  char *out_str = NULL;
  err = cmc_field_get_str(field_str, &out_str);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("whatever", out_str);

  int out_int = -1;
  err = cmc_field_get_int(field_int, &out_int);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(1, out_int);

  char *out_empty = NULL;
  err = cmc_field_get_str(field_empty, &out_empty);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("default_value", out_empty);
}
