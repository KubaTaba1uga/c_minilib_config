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

#ifndef DICT_CONFIG_PATH
#define DICT_CONFIG_PATH "non_exsistent_path"
#endif

static struct cmc_ConfigParseInterface parser;
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
                                       (char *[]){(char *)DICT_CONFIG_PATH},
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
  err = cmc_field_add_subfield(f_dict, f_name);
  TEST_ASSERT_NULL(err);

  err =
      cmc_field_create("age", cmc_ConfigFieldTypeEnum_INT, NULL, true, &f_age);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(f_dict, f_age);
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

void test_nested_dict_person_fields(void) {
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)DICT_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "dict",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *f_outer_dict, *f_inner_dict, *f_name, *f_age,
      *f_optional;

  err = cmc_field_create("nested_dict", cmc_ConfigFieldTypeEnum_DICT, NULL,
                         true, &f_outer_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("person", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &f_inner_dict);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(f_outer_dict, f_inner_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, "<none>", true,
                         &f_name);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(f_inner_dict, f_name);
  TEST_ASSERT_NULL(err);

  err =
      cmc_field_create("age", cmc_ConfigFieldTypeEnum_INT, NULL, true, &f_age);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(f_inner_dict, f_age);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("optional", cmc_ConfigFieldTypeEnum_STRING, "n/a",
                         true, &f_optional);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(f_inner_dict, f_optional);
  TEST_ASSERT_NULL(err);

  err = cmc_config_add_field(f_outer_dict, config);
  TEST_ASSERT_NULL(err);

  err = parser.parse(strlen(DICT_CONFIG_PATH), DICT_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  char *out_name = NULL, *out_optional = NULL;
  int out_age = -1;

  err = cmc_field_get_str(f_name, &out_name);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("john", out_name);

  err = cmc_field_get_int(f_age, &out_age);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(99, out_age);

  err = cmc_field_get_str(f_optional, &out_optional);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("n/a", out_optional);
}

void test_parse_list_of_persons_env_file(void) {
  // 1. Init parser & config
  err = cmc_env_parser_init(&parser);
  TEST_ASSERT_NULL(err);

  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)DICT_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "dict",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  // 2. Declare list of dicts containing dict with fields
  struct cmc_ConfigField *f_list, *f_item, *f_person, *f_name, *f_age;

  err = cmc_field_create("list", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &f_list);
  TEST_ASSERT_NULL(err);

  err = cmc_config_add_field(f_list, config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &f_item);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(f_list, f_item);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("person", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &f_person);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(f_item, f_person);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &f_name);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(f_person, f_name);
  TEST_ASSERT_NULL(err);

  err =
      cmc_field_create("age", cmc_ConfigFieldTypeEnum_INT, NULL, true, &f_age);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(f_person, f_age);
  TEST_ASSERT_NULL(err);

  // 3. Parse config
  err = parser.parse(strlen(DICT_CONFIG_PATH), DICT_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  // 4. First person: list[0] → dict > person → name/age
  TEST_ASSERT_EQUAL_INT(2, f_list->_self.subnodes_len);
  struct cmc_ConfigField *dict_0_f =
      cmc_field_of_node(f_list->_self.subnodes[0]);
  TEST_ASSERT_EQUAL_INT(1, dict_0_f->_self.subnodes_len);
  struct cmc_ConfigField *person_0_f =
      cmc_field_of_node(dict_0_f->_self.subnodes[0]);
  TEST_ASSERT_EQUAL_INT(2, person_0_f->_self.subnodes_len);
  struct cmc_ConfigField *name_0_f =
      cmc_field_of_node(person_0_f->_self.subnodes[0]);
  struct cmc_ConfigField *age_0_f =
      cmc_field_of_node(person_0_f->_self.subnodes[1]);

  char *out_s = NULL;
  int out_i = -1;

  err = cmc_field_get_str(name_0_f, &out_s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("john", out_s);

  err = cmc_field_get_int(age_0_f, &out_i);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(99, out_i);

  // 5. Second person: list[1] → dict -> person → name/age
  struct cmc_ConfigField *dict_1_f =
      cmc_field_of_node(f_list->_self.subnodes[1]);
  TEST_ASSERT_EQUAL_INT(1, dict_1_f->_self.subnodes_len);
  struct cmc_ConfigField *person_1_f =
      cmc_field_of_node(dict_1_f->_self.subnodes[0]);
  TEST_ASSERT_EQUAL_INT(2, person_1_f->_self.subnodes_len);
  struct cmc_ConfigField *name_1_f =
      cmc_field_of_node(person_1_f->_self.subnodes[0]);
  struct cmc_ConfigField *age_1_f =
      cmc_field_of_node(person_1_f->_self.subnodes[1]);

  err = cmc_field_get_str(name_1_f, &out_s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("alan", out_s);

  err = cmc_field_get_int(age_1_f, &out_i);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(47, out_i);
}
