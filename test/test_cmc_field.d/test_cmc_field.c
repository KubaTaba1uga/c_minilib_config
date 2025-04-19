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
  TEST_ASSERT_EQUAL_STRING("default", (char *)field->default_value);
}

void test_field_add_string_value(void) {
  test_field_create_string_with_default();
  err = cmc_field_add_value_str(field, "value");
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_NOT_NULL(field->value);
  TEST_ASSERT_EQUAL_STRING("value", (char *)field->value);
}

void test_field_add_string_value_twice_fails(void) {
  test_field_add_string_value();
  err = cmc_field_add_value_str(field, "another");
  TEST_ASSERT_NOT_NULL(err);
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
  TEST_ASSERT_NOT_NULL(field->default_value);
  TEST_ASSERT_EQUAL_INT(val, *(int32_t *)field->default_value);
}

void test_field_add_int_value(void) {
  int32_t val = 0;
  err =
      cmc_field_create("port", cmc_ConfigFieldTypeEnum_INT, &val, true, &field);
  TEST_ASSERT_NULL(err);

  err = cmc_field_add_value_int(field, 8080);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(8080, *(int32_t *)field->value);
}

void test_field_add_int_value_twice_fails(void) {
  test_field_add_int_value();
  err = cmc_field_add_value_int(field, 9090);
  TEST_ASSERT_NOT_NULL(err);
}
