#include <stdio.h>
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

#define FIELD_NAME_MAX 32
#define TEST_LOOPS_MAX 10
void test_cmc_config_add_valid_field(void) {
  err = cmc_config_create(NULL, &config);
  TEST_ASSERT_NULL(err);

  for (int i = 0; i < TEST_LOOPS_MAX; i++) {
    char name[FIELD_NAME_MAX];
    sprintf(name, "server_host_%d", i);

    err = cmc_config_add_field(
        &(struct cmc_ConfigField){.name = name,
                                  .type = cmc_ConfigFieldTypeEnum_STRING,
                                  .default_value = "localhost",
                                  .optional = true,
                                  .next_field = NULL},
        config);

    TEST_ASSERT_NULL(err);
    TEST_ASSERT_NOT_NULL(config->fields);
    TEST_ASSERT_EQUAL_STRING(name, config->fields->name);
    TEST_ASSERT_EQUAL_STRING("localhost", config->fields->default_value);
    TEST_ASSERT_TRUE(config->fields->optional);
  }
}
