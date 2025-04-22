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

#ifndef ARRAY_CONFIG_PATH
#define ARRAY_CONFIG_PATH "non_exsistent_path"
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
void test_flat_array_parsing(void) {
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){
          .supported_paths = (char *[]){(char *)ARRAY_CONFIG_PATH},
          .paths_length    = 1,
          .name            = "array",
          .log_func        = NULL,
      },
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *f_array, *f_elem;
  err = cmc_field_create("flat_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &f_array);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &f_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(f_array, f_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_array, config);
  TEST_ASSERT_NULL(err);

  err = parser.parse(strlen(ARRAY_CONFIG_PATH), ARRAY_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  const char *expected[] = { "a", "b", "c", "d" };
  // there should be 4 elements in the subnodes array
  TEST_ASSERT_EQUAL_UINT32(4, f_array->_self.subnodes_len);

  for (uint32_t i = 0; i < f_array->_self.subnodes_len; ++i) {
    struct cmc_TreeNode *node = f_array->_self.subnodes[i];
    struct cmc_ConfigField *elem = cmc_field_of_node(node);
    char *out = NULL;
    err = cmc_field_get_str(elem, &out);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_STRING(expected[i], out);
  }
}

void test_required_array_without_default_should_fail_always(void) {
    /* Setup config for nonexistent input */
    err = cmc_config_create(
        &(struct cmc_ConfigSettings){
            .supported_paths = (char *[]){ (char *)ARRAY_CONFIG_PATH },
            .paths_length    = 1,
            .name            = "array",
            .log_func        = NULL,
        },
        &config
    );
    TEST_ASSERT_NULL(err);

    /* Declare required array of integers (no default) */
    struct cmc_ConfigField *f_required_array = NULL;
    struct cmc_ConfigField *f_elem           = NULL;
    err = cmc_field_create(
        "required_array",
        cmc_ConfigFieldTypeEnum_ARRAY,
        NULL,
        false,               /* array itself is required */
        &f_required_array
    );
    TEST_ASSERT_NULL(err);

    /* this is just the element‐type template */
    err = cmc_field_create(
        "",
        cmc_ConfigFieldTypeEnum_INT,
        NULL,
        true,                /* element optionality will be overridden */
        &f_elem
    );
    TEST_ASSERT_NULL(err);

    /* attach the element template to our array */
    err = cmc_field_add_subfield(f_required_array, f_elem);
    TEST_ASSERT_NULL(err);

    /* register the array in the config */
    err = cmc_config_add_field(f_required_array, config);
    TEST_ASSERT_NULL(err);

    /* now parse—since there is no required_array_0 in the .env, this should fail */
    err = parser.parse(
        strlen(ARRAY_CONFIG_PATH),
        ARRAY_CONFIG_PATH,
        NULL,       /* env‐parser ignores this anyway */
        config
    );
    TEST_ASSERT_NOT_NULL(err);
    TEST_ASSERT_EQUAL_INT(ENODATA, err->code);
}

void test_optional_array_parsing(void) {
    /* create config */
    err = cmc_config_create(
        &(struct cmc_ConfigSettings){
            .supported_paths = (char *[]){(char *)ARRAY_CONFIG_PATH},
            .paths_length    = 1,
            .name            = "array",
            .log_func        = NULL,
        },
        &config
    );
    TEST_ASSERT_NULL(err);

    /* declare OPTIONAL_ARRAY as an ARRAY of STRING */
    struct cmc_ConfigField *f_array = NULL, *f_elem = NULL;
    err = cmc_field_create("optional_array",
                           cmc_ConfigFieldTypeEnum_ARRAY,
                           NULL,  /* no default */
                           true,  /* optional */
                           &f_array);
    TEST_ASSERT_NULL(err);
    err = cmc_field_create("",
                           cmc_ConfigFieldTypeEnum_STRING,
                           NULL,
                           true,
                           &f_elem);
    TEST_ASSERT_NULL(err);

    /* attach element‐template to the array */
    err = cmc_field_add_subfield(f_array, f_elem);
    TEST_ASSERT_NULL(err);

    /* register array */
    err = cmc_config_add_field(f_array, config);
    TEST_ASSERT_NULL(err);

    /* parse—should succeed, but produce no values */
    err = parser.parse(strlen(ARRAY_CONFIG_PATH),
                       ARRAY_CONFIG_PATH,
                       NULL,
                       config);
    TEST_ASSERT_NULL(err);

    /* there should be at least one element‐node under the array */
    TEST_ASSERT_TRUE(f_array->_self.subnodes_len > 0);

    /* grab the first element node */
    struct cmc_TreeNode *first_node = f_array->_self.subnodes[0];
    struct cmc_ConfigField *elem = cmc_field_of_node(first_node);

    /* it exists but has no value set */
    TEST_ASSERT_NULL(elem->value);

    /* and it has no further children */
    TEST_ASSERT_EQUAL_UINT32(0, elem->_self.subnodes_len);
}

void test_nested_array_parsing(void) {
    /* create config */
    err = cmc_config_create(
        &(struct cmc_ConfigSettings){
            .supported_paths = (char *[]){(char *)ARRAY_CONFIG_PATH},
            .paths_length    = 1,
            .name            = "array",
            .log_func        = NULL,
        },
        &config
    );
    TEST_ASSERT_NULL(err);

    /* declare nested_array: ARRAY→ARRAY→INT */
    struct cmc_ConfigField *f_outer = NULL, *f_inner = NULL, *f_int = NULL;
    err = cmc_field_create("nested_array",
                           cmc_ConfigFieldTypeEnum_ARRAY,
                           NULL,
                           true,
                           &f_outer);
    TEST_ASSERT_NULL(err);

    /* register in config */
    err = cmc_config_add_field(f_outer, config);
    TEST_ASSERT_NULL(err);

    
    err = cmc_field_create("",
                           cmc_ConfigFieldTypeEnum_ARRAY,
                           NULL,
                           true,
                           &f_inner);
    TEST_ASSERT_NULL(err);

    /* attach inner level */
    err = cmc_field_add_subfield(f_outer, f_inner);
    TEST_ASSERT_NULL(err);
    
    err = cmc_field_create("",
                           cmc_ConfigFieldTypeEnum_INT,
                           NULL,
                           true,
                           &f_int);
    TEST_ASSERT_NULL(err);

    /* attach value level */    
    err = cmc_field_add_subfield(f_inner, f_int);
    TEST_ASSERT_NULL(err);

    /* parse */
    err = parser.parse(strlen(ARRAY_CONFIG_PATH),
                       ARRAY_CONFIG_PATH,
                       NULL,
                       config);
    TEST_ASSERT_NULL(err);

    /* verify values */
    int expected[2][2] = {{0,1},{10,11}};
    /* top‐level array has 2 rows */
    TEST_ASSERT_EQUAL_UINT32(2, f_outer->_self.subnodes_len);
    for (uint32_t i = 0; i < f_outer->_self.subnodes_len; ++i) {
        struct cmc_TreeNode *row_node = f_outer->_self.subnodes[i];
        struct cmc_ConfigField *row = cmc_field_of_node(row_node);
        /* each row has 2 cells */
        TEST_ASSERT_EQUAL_UINT32(2, row->_self.subnodes_len);
        for (uint32_t j = 0; j < row->_self.subnodes_len; ++j) {
            struct cmc_TreeNode *cell_node = row->_self.subnodes[j];
            struct cmc_ConfigField *cell = cmc_field_of_node(cell_node);
            int val = -1;
            err = cmc_field_get_int(cell, &val);
            TEST_ASSERT_NULL(err);
            TEST_ASSERT_EQUAL_INT(expected[i][j], val);
        }
    }
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

  // use cmc_field_add_subfield, not the nonexistent nested function
  err = cmc_field_add_subfield(inner, leaf);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(mid, inner);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(outer, mid);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(outer, config);
  TEST_ASSERT_NULL(err);

  err = parser.parse(strlen(ARRAY_CONFIG_PATH), ARRAY_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  // we expect exactly two rows at the top level
  TEST_ASSERT_EQUAL_UINT32(2, outer->_self.subnodes_len);

  // probing values:
  for (uint32_t i = 0; i < outer->_self.subnodes_len; ++i) {
    struct cmc_ConfigField *lvl1 =
        cmc_field_of_node(outer->_self.subnodes[i]);
    for (uint32_t j = 0; j < lvl1->_self.subnodes_len; ++j) {
      struct cmc_ConfigField *lvl2 =
          cmc_field_of_node(lvl1->_self.subnodes[j]);
      for (uint32_t k = 0; k < lvl2->_self.subnodes_len; ++k) {
        struct cmc_ConfigField *lvl3 =
            cmc_field_of_node(lvl2->_self.subnodes[k]);
        int val = -1;
        err = cmc_field_get_int(lvl3, &val);
        TEST_ASSERT_NULL(err);

        if (i == 0) {
          // first outer: [ {0,1,2}, {10,11} ]
          if (j == 0) {
            TEST_ASSERT_EQUAL_INT((int)k, val);
          } else {
            TEST_ASSERT_EQUAL_INT(10 + (int)k, val);
          }
        } else {
          // second outer: [ {100}, {110}, {120} ]
          TEST_ASSERT_EQUAL_INT(100 + 10 * (int)j, val);
        }
      }
    }
  }
}
