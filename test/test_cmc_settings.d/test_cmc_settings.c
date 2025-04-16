#include <c_minilib_config.h>

#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "utils/cmc_settings.h"

static struct cmc_ConfigSettings *settings;
static cmc_error_t err;


void setUp(void){
  settings = NULL;
  err = NULL;
}

void tearDown(void){
  cmc_settings_destroy(&settings);
  cmc_error_destroy(&err);  
}

void test_cmc_settings_create_basic(void) {
  const char *paths[] = {"/etc/app", "/home/user/"};
  const uint32_t paths_len = sizeof(paths) / sizeof(char *);
  const char *name = "app_config";

 err =
      cmc_settings_create(paths_len, paths, name, NULL, &settings);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(settings);
  TEST_ASSERT_EQUAL_UINT32(paths_len, settings->paths_length);
  TEST_ASSERT_EQUAL_STRING(name, settings->name);

  for (uint32_t i = 0; i < paths_len; ++i) {
    TEST_ASSERT_EQUAL_STRING(paths[i], settings->supported_paths[i]);
  }
}

void test_cmc_settings_create_null_paths(void) {
  err = cmc_settings_create(0, NULL, NULL, NULL, &settings);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(settings);
  TEST_ASSERT_EQUAL_UINT32(0, settings->paths_length);
  TEST_ASSERT_NOT_NULL(settings->name);
  TEST_ASSERT_EQUAL_STRING("config", settings->name);
}

void test_cmc_settings_create_invalid_output(void) {
  err = cmc_settings_create(0, NULL, NULL, NULL, NULL);
  TEST_ASSERT_NOT_NULL(err);
}
