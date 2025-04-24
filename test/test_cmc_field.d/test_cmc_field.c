#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "c_minilib_config.h"
#include "utils/cmc_error.h"
#include "utils/cmc_field.h"

// Global parent fields only
static struct cmc_ConfigField *field = NULL;
static struct cmc_ConfigField *parent = NULL;
static struct cmc_ConfigField *arr = NULL;
static struct cmc_ConfigField *dict = NULL;

static cmc_error_t err = NULL;

void setUp(void) {
  field = parent = arr = dict = NULL;
  err = NULL;
}

void tearDown(void) {
  cmc_field_destroy(&field);
  cmc_field_destroy(&parent);
  cmc_field_destroy(&arr);
  cmc_field_destroy(&dict);
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
  err = cmc_field_create("my_field", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &field);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_value_str(field, "value");
  TEST_ASSERT_NULL(err);
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
  err = cmc_field_add_value_int(field, 333);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(333, *(int32_t *)field->value);
}

void test_field_add_valid_subfield(void) {
  struct cmc_ConfigField *child = NULL;
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
}

void test_field_add_subfield_to_invalid_type_should_fail(void) {
  struct cmc_ConfigField *non_container = NULL, *child = NULL;
  cmc_field_create("not_a_container", cmc_ConfigFieldTypeEnum_STRING, NULL,
                   true, &non_container);
  cmc_field_create("child", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &child);
  err = cmc_field_add_subfield(non_container, child);
  TEST_ASSERT_NOT_NULL(err);
  cmc_field_destroy(&non_container);
  cmc_field_destroy(&child);
}

void test_field_destroy_with_subfields(void) {
  struct cmc_ConfigField *child = NULL;
  err = cmc_field_create("dict", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &parent);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("key", cmc_ConfigFieldTypeEnum_STRING, "abc", true,
                         &child);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(parent, child);
  TEST_ASSERT_NULL(err);
}

void test_field_destroy_with_nested_subfields(void) {
  struct cmc_ConfigField *array = NULL, *str = NULL;
  err = cmc_field_create("outer", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &parent);
  TEST_ASSERT_NULL(err);
  cmc_field_create("nested", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true, &array);
  cmc_field_create("str", cmc_ConfigFieldTypeEnum_STRING, "abc", true, &str);
  cmc_field_add_subfield(array, str);
  cmc_field_add_subfield(parent, array);
}

void test_iter_array_macro_should_visit_all_elements(void) {
  struct cmc_ConfigField *elem1 = NULL, *elem2 = NULL;
  err = cmc_field_create("array", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &arr);
  TEST_ASSERT_NULL(err);
  cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &elem1);
  cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &elem2);
  cmc_field_add_value_str(elem1, "hello");
  cmc_field_add_value_str(elem2, "world");
  cmc_field_add_subfield(arr, elem1);
  cmc_field_add_subfield(arr, elem2);

  int count = 0;
  CMC_FIELD_ITER_ARRAY(val, char *, &arr, {
    if (count == 0) {
      TEST_ASSERT_EQUAL_STRING("hello", val);
    } else if (count == 1) {
      TEST_ASSERT_EQUAL_STRING("world", val);
    }
    count++;
  });
  TEST_ASSERT_EQUAL_INT(2, count);
}

void test_iter_dict_macro_should_visit_all_entries(void) {
  struct cmc_ConfigField *f1 = NULL, *f2 = NULL;
  err =
      cmc_field_create("dict", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &dict);
  TEST_ASSERT_NULL(err);
  cmc_field_create("key1", cmc_ConfigFieldTypeEnum_INT, NULL, true, &f1);
  cmc_field_create("key2", cmc_ConfigFieldTypeEnum_INT, NULL, true, &f2);
  cmc_field_add_value_int(f1, 10);
  cmc_field_add_value_int(f2, 20);
  cmc_field_add_subfield(dict, f1);
  cmc_field_add_subfield(dict, f2);

  int sum = 0, matched = 0;
  CMC_FIELD_ITER_DICT(val, int *, &dict, {
    if (strcmp(val_name, "key1") == 0) {
      TEST_ASSERT_EQUAL_INT(10, *val);
      matched++;
    } else if (strcmp(val_name, "key2") == 0) {
      TEST_ASSERT_EQUAL_INT(20, *val);
      matched++;
    }
    sum += *val;
  });
  TEST_ASSERT_EQUAL_INT(30, sum);
  TEST_ASSERT_EQUAL_INT(2, matched);
}

void test_iter_array_macro_should_visit_all_nested_elements(void) {
  struct cmc_ConfigField *elem1 = NULL, *elem2 = NULL, *nested = NULL,
                         *nested_elem = NULL;

  // Create the main array
  err = cmc_field_create("array", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &arr);
  TEST_ASSERT_NULL(err);

  // Flat elements
  cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &elem1);
  cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &elem2);
  cmc_field_add_value_str(elem1, "hello");
  cmc_field_add_value_str(elem2, "world");
  cmc_field_add_subfield(arr, elem1);
  cmc_field_add_subfield(arr, elem2);

  // Nested array inside the main array
  cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true, &nested);
  cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &nested_elem);
  cmc_field_add_value_str(nested_elem, "nested-value");
  cmc_field_add_subfield(nested, nested_elem);
  cmc_field_add_subfield(arr, nested);

  int count = 0;
  CMC_FIELD_ITER_ARRAY(val, void *, &arr, {
    struct cmc_ConfigField *subfield = cmc_field_of_node(__valsubnode);
    switch (subfield->type) {
    case cmc_ConfigFieldTypeEnum_STRING: {
      if (count == 0) {
        TEST_ASSERT_EQUAL_STRING("hello", (char *)val);
      } else if (count == 1) {
        TEST_ASSERT_EQUAL_STRING("world", (char *)val);
      }
      break;
    }
    case cmc_ConfigFieldTypeEnum_ARRAY: {
      CMC_FIELD_ITER_ARRAY(nval, char *, &subfield,
                           { TEST_ASSERT_EQUAL_STRING("nested-value", nval); });
      break;
    }
    default: {
    }
    }
    count++;
  });

  TEST_ASSERT_EQUAL_INT(3, count); // 2 flat + 1 nested
}
