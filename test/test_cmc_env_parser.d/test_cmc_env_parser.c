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

void test_parse_array_env_file(void) {
  // 1) create config for directory ARRAY_CONFIG_PATH and base name "array"
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)ARRAY_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "array",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  // 2) empty_array: declare ARRAY of INT
  struct cmc_ConfigField *f_empty_array, *f_empty_elem;
  err = cmc_field_create("empty_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &f_empty_array);
  TEST_ASSERT_NULL(err);
  // nested INT descriptor
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_INT, NULL, true,
                         &f_empty_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_empty_array, f_empty_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_empty_array, config);
  TEST_ASSERT_NULL(err);

  // 3) filled_array: same setup
  struct cmc_ConfigField *f_filled_array, *f_filled_elem;
  err = cmc_field_create("filled_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &f_filled_array);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_INT, NULL, true,
                         &f_filled_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_filled_array, f_filled_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_filled_array, config);
  TEST_ASSERT_NULL(err);

  // 4) nested_array: ARRAY of ARRAY of INT
  struct cmc_ConfigField *f_nested_array, *f_nested_row, *f_nested_cell;
  err = cmc_field_create("nested_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &f_nested_array);
  TEST_ASSERT_NULL(err);

  // first level: rows (ARRAY)
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &f_nested_row);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_nested_array, f_nested_row);
  TEST_ASSERT_NULL(err);
  // second level: cells (INT)
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_INT, NULL, true,
                         &f_nested_cell);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_nested_row, f_nested_cell);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_nested_array, config);
  TEST_ASSERT_NULL(err);

  // 5) parse
  err =
      parser.parse(strlen(ARRAY_CONFIG_PATH), ARRAY_CONFIG_PATH, NULL, config);
  /* puts(err->msg); */
  TEST_ASSERT_NULL(err);

  // 6) check empty_array: first element should be optional 0, no next
  struct cmc_ConfigField *elem = f_empty_array->value;
  int out;
  err = cmc_field_get_int(elem, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(0, out);
  TEST_ASSERT_NULL(elem->next_field);

  // 7) check filled_array: expect [0,1]
  elem = f_filled_array->value;
  err = cmc_field_get_int(elem, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(0, out);
  elem = elem->next_field;
  TEST_ASSERT_NOT_NULL(elem);
  err = cmc_field_get_int(elem, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(1, out);
  TEST_ASSERT_NULL(elem->next_field);

  // 8) check nested_array: 2×2 matrix: [ [0,1], [10,11] ]
  struct cmc_ConfigField *row = f_nested_array->value;
  // row 0
  struct cmc_ConfigField *cell = row->value;
  err = cmc_field_get_int(cell, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(0, out);
  cell = cell->next_field;
  err = cmc_field_get_int(cell, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(1, out);
  // row 1
  row = row->next_field;
  TEST_ASSERT_NOT_NULL(row);
  cell = row->value;
  TEST_ASSERT_NOT_NULL(cell);
  err = cmc_field_get_int(cell, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(10, out);
  cell = cell->next_field;
  err = cmc_field_get_int(cell, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(11, out);
}

void test_required_array_without_default_should_fail(void) {
  // Setup config for nonexistent input
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)ARRAY_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "array",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  // Declare required array of integers (no default)
  struct cmc_ConfigField *f_required_array, *f_elem;
  err = cmc_field_create("required_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         false, &f_required_array);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_INT, NULL, false,
                         &f_elem); // required element
  TEST_ASSERT_NULL(err);

  err = cmc_field_add_nested_field(f_required_array, f_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_required_array, config);
  TEST_ASSERT_NULL(err);

  // Parse: expect error due to missing required array entries
  err =
      parser.parse(strlen(ARRAY_CONFIG_PATH), ARRAY_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NOT_NULL(err);
  TEST_ASSERT_EQUAL_INT(ENOENT, err->code);
}

void test_required_array_without_default_should_fail_always(void) {
  // Setup config for nonexistent input
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)ARRAY_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "array",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  // Declare required array of integers (no default)
  struct cmc_ConfigField *f_required_array, *f_elem;
  err = cmc_field_create("required_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         false, &f_required_array);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_INT, NULL, true,
                         &f_elem); // required element
  TEST_ASSERT_NULL(err);

  err = cmc_field_add_nested_field(f_required_array, f_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_required_array, config);
  TEST_ASSERT_NULL(err);

  // Parse: expect error due to missing required array entries
  err =
      parser.parse(strlen(ARRAY_CONFIG_PATH), ARRAY_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NOT_NULL(err);
  TEST_ASSERT_EQUAL_INT(ENOENT, err->code);
}

/* void test_array_nested_env_config(void) { */
/*   err = cmc_config_create( */
/*       &(struct cmc_ConfigSettings){ */
/*           .supported_paths = (char *[]){(char *)ARRAY_NESTED_CONFIG_PATH}, */
/*           .paths_length = 1, */
/*           .name = "array_nested", */
/*           .log_func = NULL}, */
/*       &config); */
/*   TEST_ASSERT_NULL(err); */

/*   // envs */
/*   struct cmc_ConfigField *f_envs, *f_envs_elem; */
/*   err = cmc_field_create("envs", cmc_ConfigFieldTypeEnum_ARRAY, NULL, false,
 */
/*                          &f_envs); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, false, */
/*                          &f_envs_elem); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_add_nested_field(f_envs, f_envs_elem); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_config_add_field(f_envs, config); */
/*   TEST_ASSERT_NULL(err); */

/*   // max_connections */
/*   struct cmc_ConfigField *f_max; */
/*   err = cmc_field_create("max_connections", cmc_ConfigFieldTypeEnum_INT,
 * NULL, */
/*                          false, &f_max); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_config_add_field(f_max, config); */
/*   TEST_ASSERT_NULL(err); */

/*   // retry_delays */
/*   struct cmc_ConfigField *f_retries, *f_retries_elem; */
/*   err = cmc_field_create("retry_delays", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
 */
/*                          false, &f_retries); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_create("", cmc_ConfigFieldTypeEnum_INT, NULL, false, */
/*                          &f_retries_elem); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_add_nested_field(f_retries, f_retries_elem); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_config_add_field(f_retries, config); */
/*   TEST_ASSERT_NULL(err); */

/*   // clusters: 5-level nested string */
/*   struct cmc_ConfigField *f_clusters, *lvl1, *lvl2, *lvl3, *lvl4; */
/*   err = cmc_field_create("clusters", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
 * false, */
/*                          &f_clusters); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, false,
 * &lvl1); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, false,
 * &lvl2); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, false,
 * &lvl3); */
/*   TEST_ASSERT_NULL(err); */
/*   err = */
/*       cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, false,
 * &lvl4); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_add_nested_field(lvl3, lvl4); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_add_nested_field(lvl2, lvl3); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_add_nested_field(lvl1, lvl2); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_add_nested_field(f_clusters, lvl1); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_config_add_field(f_clusters, config); */
/*   TEST_ASSERT_NULL(err); */

/*   // time_windows */
/*   struct cmc_ConfigField *f_windows, *f_row, *f_cell; */
/*   err = cmc_field_create("time_windows", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
 */
/*                          false, &f_windows); */
/*   TEST_ASSERT_NULL(err); */
/*   err = */
/*       cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, false,
 * &f_row); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, false, */
/*                          &f_cell); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_add_nested_field(f_row, f_cell); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_field_add_nested_field(f_windows, f_row); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_config_add_field(f_windows, config); */
/*   TEST_ASSERT_NULL(err); */

/*   // enable_metrics */
/*   struct cmc_ConfigField *f_metrics; */
/*   err = cmc_field_create("enable_metrics", cmc_ConfigFieldTypeEnum_INT, NULL,
 */
/*                          false, &f_metrics); */
/*   TEST_ASSERT_NULL(err); */
/*   err = cmc_config_add_field(f_metrics, config); */
/*   TEST_ASSERT_NULL(err); */

/*   // Parse */
/*   err = parser.parse(strlen(ARRAY_NESTED_CONFIG_PATH),
 * ARRAY_NESTED_CONFIG_PATH, */
/*                      NULL, config); */
/*   puts(err->msg); */
/*   TEST_ASSERT_NULL(err); */

/*   // Validate envs */
/*   const char *expected_envs[] = {"dev", "staging", "prod"}; */
/*   struct cmc_ConfigField *it = f_envs->value; */
/*   for (int i = 0; i < 3; ++i) { */
/*     char *val = NULL; */
/*     TEST_ASSERT_NOT_NULL(it); */
/*     err = cmc_field_get_str(it, &val); */
/*     TEST_ASSERT_NULL(err); */
/*     TEST_ASSERT_EQUAL_STRING(expected_envs[i], val); */
/*     it = it->next_field; */
/*   } */

/*   // Validate retry_delays */
/*   int expected_retries[] = {1, 2, 5}; */
/*   it = f_retries->value; */
/*   for (int i = 0; i < 3; ++i) { */
/*     int val = -1; */
/*     TEST_ASSERT_NOT_NULL(it); */
/*     err = cmc_field_get_int(it, &val); */
/*     TEST_ASSERT_NULL(err); */
/*     TEST_ASSERT_EQUAL_INT(expected_retries[i], val); */
/*     it = it->next_field; */
/*   } */

/*   // Validate max_connections */
/*   int val = -1; */
/*   err = cmc_field_get_int(f_max, &val); */
/*   TEST_ASSERT_NULL(err); */
/*   TEST_ASSERT_EQUAL_INT(1000, val); */

/*   // Validate enable_metrics */
/*   err = cmc_field_get_int(f_metrics, &val); */
/*   TEST_ASSERT_NULL(err); */
/*   TEST_ASSERT_EQUAL_INT(1, val); */

/*   // Validate time_windows */
/*   const char *expected_windows[2][2] = {{"08:00", "12:00"}, {"14:00",
 * "18:00"}}; */
/*   struct cmc_ConfigField *row = f_windows->value; */
/*   for (int i = 0; i < 2; ++i) { */
/*     TEST_ASSERT_NOT_NULL(row); */
/*     struct cmc_ConfigField *cell = row->value; */
/*     for (int j = 0; j < 2; ++j) { */
/*       TEST_ASSERT_NOT_NULL(cell); */
/*       char *val_s = NULL; */
/*       err = cmc_field_get_str(cell, &val_s); */
/*       TEST_ASSERT_NULL(err); */
/*       TEST_ASSERT_EQUAL_STRING(expected_windows[i][j], val_s); */
/*       cell = cell->next_field; */
/*     } */
/*     row = row->next_field; */
/*   } */

/*   // Validate clusters */
/*   const char *expected_cluster = "cannary"; */
/*   struct cmc_ConfigField *l1 = f_clusters->value; */
/*   TEST_ASSERT_NOT_NULL(l1); */
/*   struct cmc_ConfigField *l2 = l1->value; */
/*   TEST_ASSERT_NOT_NULL(l2); */
/*   struct cmc_ConfigField *l3 = l2->value; */
/*   TEST_ASSERT_NOT_NULL(l3); */
/*   struct cmc_ConfigField *l4 = l3->value; */
/*   TEST_ASSERT_NOT_NULL(l4); */
/*   struct cmc_ConfigField *l5 = l4->value; */
/*   TEST_ASSERT_NOT_NULL(l5); */
/*   char *out_s = NULL; */
/*   err = cmc_field_get_str(l5, &out_s); */
/*   TEST_ASSERT_NULL(err); */
/*   TEST_ASSERT_EQUAL_STRING(expected_cluster, out_s); */
/* } */
