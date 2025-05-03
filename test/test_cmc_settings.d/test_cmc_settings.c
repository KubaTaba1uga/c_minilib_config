/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unity.h>

#include "c_minilib_config.h"
#include "c_minilib_error.h"
#include "utils/cmc_settings.h"

static struct cmc_ConfigSettings *settings;
static cme_error_t err;

void setUp(void) {
  cme_init();
  settings = NULL;
  err = NULL;
}

void tearDown(void) {
  cmc_settings_destroy(&settings);
  cme_destroy();
}

void test_cmc_settings_create_basic(void) {
  const char *paths[] = {"/etc/app", "/home/user/"};
  const uint32_t paths_len = sizeof(paths) / sizeof(char *);
  const char *name = "app_config";

  err = cmc_settings_create(paths_len, paths, name, NULL, &settings);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(settings);
  TEST_ASSERT_EQUAL_UINT32(paths_len + 1, settings->paths_length);
  TEST_ASSERT_EQUAL_STRING(name, settings->name);

  char **supported_paths = settings->supported_paths + 1;
  for (uint32_t i = 0; i < paths_len; ++i) {
    TEST_ASSERT_EQUAL_STRING(paths[i], supported_paths[i]);
  }
}

void test_cmc_settings_create_null_paths(void) {
  err = cmc_settings_create(0, NULL, NULL, NULL, &settings);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(settings);
  // Default search path is current working dir
  TEST_ASSERT_EQUAL_UINT32(1, settings->paths_length);
  TEST_ASSERT_NOT_NULL(settings->name);
  TEST_ASSERT_EQUAL_STRING("config", settings->name);

  for (uint32_t i = 0; i < settings->paths_length; ++i) {
    char buffer[PATH_MAX];
    TEST_ASSERT_EQUAL_STRING(getcwd(buffer, PATH_MAX),
                             settings->supported_paths[i]);
  }
}

void test_cmc_settings_create_invalid_output(void) {
  err = cmc_settings_create(0, NULL, NULL, NULL, NULL);
  TEST_ASSERT_NOT_NULL(err);
}
