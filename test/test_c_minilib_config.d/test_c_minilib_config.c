#include <unity.h>

#include "c_minilib_config.h"
#include "utils/cmc_error.h"
#include "utils/cmc_field.h"
#include "utils/cmc_settings.h"

static struct cmc_Config *config = NULL;
static cmc_error_t err = NULL;

void setUp(void) {
  config = NULL;
  err = NULL;
}

void tearDown(void) {
  cmc_config_destroy(&config);
  cmc_error_destroy(&err);
}

void test_cmc_config_create_null_output(void) {
  err = cmc_config_create(NULL, NULL);
  TEST_ASSERT_NOT_NULL(err);
}

void test_cmc_config_create_with_null_settings(void) {
  err = cmc_config_create(NULL, &config);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(config);
  TEST_ASSERT_NOT_NULL(config->settings);
}

void test_cmc_config_create_with_valid_settings(void) {
  char *paths[] = {"/path/one", "/path/two"};

  err = cmc_config_create(&(struct cmc_ConfigSettings){.supported_paths = paths,
                                                       .paths_length = 2,
                                                       .name = "test_config",
                                                       .log_func = NULL},
                          &config);

  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(config);
  TEST_ASSERT_NOT_NULL(config->settings);
  TEST_ASSERT_EQUAL_UINT32(2, config->settings->paths_length);
  TEST_ASSERT_EQUAL_STRING("test_config", config->settings->name);
}

void test_cmc_config_add_field_null_args(void) {
  err = cmc_config_add_field(NULL, NULL);
  TEST_ASSERT_NOT_NULL(err);
}

void test_cmc_config_add_valid_field(void) {
  err = cmc_config_create(NULL, &config);
  TEST_ASSERT_NULL(err);

  err = cmc_config_add_field(
      &(struct cmc_ConfigField){.name = "server_host",
                                .type = cmc_ConfigFieldTypeEnum_STRING,
                                .default_value = "localhost",
                                .optional = true,
                                .next_field = NULL},
      config);

  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(config->fields);
  TEST_ASSERT_EQUAL_STRING("server_host", config->fields->name);
}
