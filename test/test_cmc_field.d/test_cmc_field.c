#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "c_minilib_config.h"
#include "utils/cmc_error.h"
#include "utils/cmc_field.h"

static struct cmc_ConfigField *field = NULL;
static cmc_error_t err = NULL;

void setUp(void) {
  field = NULL;
  err = NULL;
}

void tearDown(void) {
  cmc_field_destroy(&field);
  cmc_error_destroy(&err);
}

void test_field_create_null_args(void) {
  err =
      cmc_field_create(NULL, cmc_ConfigFieldTypeEnum_STRING, "x", true, &field);
  TEST_ASSERT_NOT_NULL(err);
}

void test_field_create_invalid_type(void) {
  err = cmc_field_create("abc", -1, "x", true, &field);
  TEST_ASSERT_NOT_NULL(err);
}

void test_field_create_optional_without_default(void) {
  err = cmc_field_create("abc", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &field);
  TEST_ASSERT_NULL(err);
}

void test_field_create_string_with_default(void) {
  err = cmc_field_create("my_field", cmc_ConfigFieldTypeEnum_STRING, "default",
                         true, &field);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(field);
  TEST_ASSERT_EQUAL_STRING("my_field", field->name);
  TEST_ASSERT_TRUE(field->optional);
  TEST_ASSERT_EQUAL_INT(cmc_ConfigFieldTypeEnum_STRING, field->type);
  TEST_ASSERT_NOT_NULL(field->value);
  TEST_ASSERT_EQUAL_STRING("default", (char *)field->value);
}

void test_field_add_string_value(void) {
  err = cmc_field_create("my_field", cmc_ConfigFieldTypeEnum_STRING, "default",
                         true, &field);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(field);

  err = cmc_field_add_value_str(field, "value");
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(field->value);
  TEST_ASSERT_EQUAL_STRING("value", (char *)field->value);
}

void test_field_add_value_null(void) {
  err = cmc_field_add_value_str(NULL, NULL);
  TEST_ASSERT_NOT_NULL(err);
}

void test_field_create_int_with_default(void) {
  int32_t val = 123;
  err =
      cmc_field_create("port", cmc_ConfigFieldTypeEnum_INT, &val, true, &field);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(field->value);
  TEST_ASSERT_EQUAL_INT(val, *(int32_t *)field->value);
}

void test_field_add_int_value(void) {
  err = cmc_field_create("port", cmc_ConfigFieldTypeEnum_INT, 0, false, &field);
  TEST_ASSERT_NULL(err);

  err = cmc_field_add_value_int(field, 8080);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(8080, *(int32_t *)field->value);
}

void test_field_add_int_value_optional(void) {
  int32_t def_val = -1;
  err = cmc_field_create("port", cmc_ConfigFieldTypeEnum_INT, &def_val, true,
                         &field);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(def_val, *(int32_t *)field->value);

  err = cmc_field_add_value_int(field, 8080);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(8080, *(int32_t *)field->value);

  err = cmc_field_add_value_int(field, 333);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(333, *(int32_t *)field->value);
}

void test_field_add_valid_subfield(void) {
  struct cmc_ConfigField *parent = NULL, *child = NULL;
  err = cmc_field_create("arr", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &parent);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("elem", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &child);
  TEST_ASSERT_NULL(err);

  err = cmc_field_add_subfield(parent, child);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_UINT32(1, parent->_self.subnodes_len);
  TEST_ASSERT_EQUAL_PTR(child, cmc_field_of_node(parent->_self.subnodes[0]));

  cmc_field_destroy(&parent);
}

void test_field_add_subfield_to_invalid_type_should_fail(void) {
  struct cmc_ConfigField *non_container = NULL, *child = NULL;
  err = cmc_field_create("not_a_container", cmc_ConfigFieldTypeEnum_STRING,
                         NULL, true, &non_container);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("child", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &child);
  TEST_ASSERT_NULL(err);

  err = cmc_field_add_subfield(non_container, child);
  TEST_ASSERT_NOT_NULL(err);

  cmc_field_destroy(&non_container);
  cmc_field_destroy(&child);
}

void test_field_destroy_with_subfields(void) {
  struct cmc_ConfigField *parent = NULL, *child = NULL;
  // Create a DICT field and a child STRING field
  err = cmc_field_create("dict", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &parent);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("key", cmc_ConfigFieldTypeEnum_STRING, "abc", true,
                         &child);
  TEST_ASSERT_NULL(err);

  // Add child to parent's tree
  err = cmc_field_add_subfield(parent, child);
  TEST_ASSERT_NULL(err);

  // Destroy recursively
  cmc_field_destroy(&parent);

  // Assert top pointer cleared
  TEST_ASSERT_NULL(parent);
}

void test_field_destroy_with_nested_subfields(void) {
  struct cmc_ConfigField *parent = NULL, *array = NULL, *str = NULL;
  ;
  // Create a DICT field and a child STRING field
  err = cmc_field_create("array", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &parent);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("nested_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &array);
  TEST_ASSERT_NULL(err);

  err = cmc_field_add_subfield(parent, array);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("str", cmc_ConfigFieldTypeEnum_STRING, "abc", true,
                         &str);
  TEST_ASSERT_NULL(err);

  err = cmc_field_add_subfield(array, str);
  TEST_ASSERT_NULL(err);

  // Destroy recursively
  cmc_field_destroy(&parent);

  // Assert top pointer cleared
  TEST_ASSERT_NULL(parent);
}
