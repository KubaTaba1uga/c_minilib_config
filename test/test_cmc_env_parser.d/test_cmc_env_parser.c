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

void __test_parse_array_env_file(void) {
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
  TEST_ASSERT_NOT_NULL(elem);

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

void __test_required_array_without_default_should_fail(void) {
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

void __test_required_array_without_default_should_fail_always(void) {
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
void test_optional_array_parsing(void) {
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)ARRAY_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "array",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *f_array, *f_elem;
  err = cmc_field_create("optional_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &f_array);
  TEST_ASSERT_NULL(err);
  err =
      cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &f_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_array, f_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_array, config);
  TEST_ASSERT_NULL(err);

  err =
      parser.parse(strlen(ARRAY_CONFIG_PATH), ARRAY_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  TEST_ASSERT_NOT_NULL(f_array->value); // Array contain it's type specification
  TEST_ASSERT_NULL(f_elem->value);      // But element value is NULL
  TEST_ASSERT_NULL(f_elem->next_field); // And it doesn't hold more values
}

void test_flat_array_parsing(void) {
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)ARRAY_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "array",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *f_array, *f_elem;
  err = cmc_field_create("flat_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &f_array);
  TEST_ASSERT_NULL(err);
  err =
      cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &f_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_array, f_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_array, config);
  TEST_ASSERT_NULL(err);

  err =
      parser.parse(strlen(ARRAY_CONFIG_PATH), ARRAY_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  const char *expected[] = {"a", "b", "c", "d"};
  struct cmc_ConfigField *elem = f_array->value;
  for (int i = 0; i < 4; i++) {
    TEST_ASSERT_NOT_NULL(elem);
    char *out = NULL;
    err = cmc_field_get_str(elem, &out);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_STRING(expected[i], out);
    elem = elem->next_field;
  }
  TEST_ASSERT_NULL(elem);
}

void test_nested_array_parsing(void) {
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)ARRAY_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "array",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *f_outer, *f_inner, *f_int;
  err = cmc_field_create("nested_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &f_outer);
  TEST_ASSERT_NULL(err);
  err =
      cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true, &f_inner);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_INT, NULL, true, &f_int);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_outer, f_inner);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_inner, f_int);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_outer, config);
  TEST_ASSERT_NULL(err);

  err =
      parser.parse(strlen(ARRAY_CONFIG_PATH), ARRAY_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  int expected[2][2] = {{0, 1}, {10, 11}};
  struct cmc_ConfigField *row = f_outer->value;
  for (int i = 0; i < 2; i++) {
    TEST_ASSERT_NOT_NULL(row);
    struct cmc_ConfigField *cell = row->value;
    for (int j = 0; j < 2; j++) {
      TEST_ASSERT_NOT_NULL(cell);
      int val = -1;
      err = cmc_field_get_int(cell, &val);
      TEST_ASSERT_NULL(err);
      TEST_ASSERT_EQUAL_INT(expected[i][j], val);
      cell = cell->next_field;
    }
    TEST_ASSERT_NULL(cell);
    row = row->next_field;
  }
  TEST_ASSERT_NULL(row);
}

void test_double_nested_array_parsing(void) {
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)ARRAY_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "array",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *outer, *mid, *inner;
  err = cmc_field_create("double_nested_array", cmc_ConfigFieldTypeEnum_ARRAY,
                         NULL, true, &outer);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true, &mid);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true, &inner);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *leaf;
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_INT, NULL, true, &leaf);
  TEST_ASSERT_NULL(err);

  err = cmc_field_add_nested_field(inner, leaf);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(mid, inner);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(outer, mid);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(outer, config);
  TEST_ASSERT_NULL(err);

  err =
      parser.parse(strlen(ARRAY_CONFIG_PATH), ARRAY_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  int expected[2][3][3] = {{{0, 1, 2}, {10, 11}}, {{100}, {110}, {120}}};

  struct cmc_ConfigField *lvl1 = outer->value;
  for (int i = 0; i < 2; i++) {
    TEST_ASSERT_NOT_NULL(lvl1);
    struct cmc_ConfigField *lvl2 = lvl1->value;
    for (int j = 0; j < 3; j++) {
      if (!lvl2)
        break;
      struct cmc_ConfigField *lvl3 = lvl2->value;
      for (int k = 0; k < 3 && lvl3; k++) {
        int val;
        err = cmc_field_get_int(lvl3, &val);
        TEST_ASSERT_NULL(err);
        TEST_ASSERT_EQUAL_INT(expected[i][j][k], val);
        lvl3 = lvl3->next_field;
      }
      lvl2 = lvl2->next_field;
    }
    lvl1 = lvl1->next_field;
  }
}

void test_optional_field_from_dict_env(void) {
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)DICT_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "dict",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *field;
  err = cmc_field_create("optional", cmc_ConfigFieldTypeEnum_STRING,
                         "default_val", true, &field);
  TEST_ASSERT_NULL(err);

  err = cmc_config_add_field(field, config);
  TEST_ASSERT_NULL(err);

  err = parser.parse(strlen(DICT_CONFIG_PATH), DICT_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  char *out_val = NULL;
  err = cmc_field_get_str(field, &out_val);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("default_val", out_val);
}

void test_optional_dict_fields_dict_name_and_dict_age(void) {
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)ARRAY_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "dict",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *f_dict, *f_name, *f_age;

  err = cmc_field_create("dict", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &f_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, "<none>", true,
                         &f_name);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_dict, f_name);
  TEST_ASSERT_NULL(err);

  err =
      cmc_field_create("age", cmc_ConfigFieldTypeEnum_INT, NULL, true, &f_age);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_next_field(f_name, f_age);
  TEST_ASSERT_NULL(err);

  err = cmc_config_add_field(f_dict, config);
  TEST_ASSERT_NULL(err);

  err = parser.parse(strlen(DICT_CONFIG_PATH), DICT_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  char *out_name = NULL;
  int out_age = -1;

  err = cmc_field_get_str(f_name, &out_name);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("john", out_name);

  err = cmc_field_get_int(f_age, &out_age);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(99, out_age);
}
