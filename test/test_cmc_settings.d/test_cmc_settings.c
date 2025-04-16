#include <c_minilib_config.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "utils/cmc_settings.h"

void test_cmc_settings_create_basic(void) {
  const char *paths[] = {"/etc/app", "/home/user/"};
  const uint32_t paths_len = sizeof(paths) / sizeof(char *);
  const char *name = "app_config";

  struct cmc_ConfigSettings *settings = NULL;

  cmc_error_t err =
      cmc_settings_create(paths_len, paths, name, NULL, &settings);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(settings);
  TEST_ASSERT_EQUAL_UINT32(paths_len, settings->paths_length);
  TEST_ASSERT_EQUAL_STRING(name, settings->name);

  for (uint32_t i = 0; i < paths_len; ++i) {
    TEST_ASSERT_EQUAL_STRING(paths[i], settings->supported_paths[i]);
  }

  cmc_settings_destroy(&settings);
  TEST_ASSERT_NULL(settings);
}

void test_cmc_settings_create_null_paths(void) {
  struct cmc_ConfigSettings *settings = NULL;

  cmc_error_t err = cmc_settings_create(0, NULL, NULL, NULL, &settings);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(settings);
  TEST_ASSERT_EQUAL_UINT32(0, settings->paths_length);
  TEST_ASSERT_NOT_NULL(settings->name);
  TEST_ASSERT_EQUAL_STRING("config", settings->name);

  cmc_settings_destroy(&settings);
  TEST_ASSERT_NULL(settings);
}

void test_cmc_settings_create_invalid_output(void) {
  cmc_error_t err = cmc_settings_create(0, NULL, NULL, NULL, NULL);
  TEST_ASSERT_NOT_NULL(err);
  cmc_error_destroy(&err);
}
