
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

  // 2) OPTIONAL_ARRAY: declare ARRAY of STRING
  struct cmc_ConfigField *f_optional_array, *f_optional_elem;
  err = cmc_field_create("optional_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &f_optional_array);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &f_optional_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_optional_array, f_optional_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_optional_array, config);
  TEST_ASSERT_NULL(err);

  // 3) FLAT_ARRAY: declare ARRAY of STRING
  struct cmc_ConfigField *f_flat_array, *f_flat_elem;
  err = cmc_field_create("flat_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &f_flat_array);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &f_flat_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_flat_array, f_flat_elem);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_flat_array, config);
  TEST_ASSERT_NULL(err);

  // 4) NESTED_ARRAY: ARRAY of ARRAY of INT
  struct cmc_ConfigField *f_nested_array, *f_nested_row, *f_nested_cell;
  err = cmc_field_create("nested_array", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &f_nested_array);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &f_nested_row);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_nested_array, f_nested_row);
  TEST_ASSERT_NULL(err);
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_INT, NULL, true,
                         &f_nested_cell);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_nested_row, f_nested_cell);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_nested_array, config);
  TEST_ASSERT_NULL(err);

  // 5) DOUBLE_NESTED_ARRAY: ARRAY of ARRAY of ARRAY of INT
  struct cmc_ConfigField *f_double_array, *f_double_mid, *f_double_inner,
      *f_double_leaf;
  err = cmc_field_create("double_nested_array", cmc_ConfigFieldTypeEnum_ARRAY,
                         NULL, true, &f_double_array);
  TEST_ASSERT_NULL(err);
  // mid level
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &f_double_mid);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_double_array, f_double_mid);
  TEST_ASSERT_NULL(err);
  // inner level
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &f_double_inner);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_double_mid, f_double_inner);
  TEST_ASSERT_NULL(err);
  // leaf level
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_INT, NULL, true,
                         &f_double_leaf);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_double_inner, f_double_leaf);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_double_array, config);
  TEST_ASSERT_NULL(err);

  // 6) parse
  err =
      parser.parse(strlen(ARRAY_CONFIG_PATH), ARRAY_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  // 7) check OPTIONAL_ARRAY: element exists, but no value
  struct cmc_ConfigField *elem = f_optional_array->value;
  TEST_ASSERT_NOT_NULL(elem);
  TEST_ASSERT_NULL(elem->value);
  TEST_ASSERT_NULL(elem->next_field);

  // 8) check FLAT_ARRAY: expect ["a","b","c","d"]
  const char *expected_strs[] = {"a", "b", "c", "d"};
  elem = f_flat_array->value;
  for (int i = 0; i < 4; i++) {
    TEST_ASSERT_NOT_NULL(elem);
    char *out_s = NULL;
    err = cmc_field_get_str(elem, &out_s);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_STRING(expected_strs[i], out_s);
    elem = elem->next_field;
  }
  TEST_ASSERT_NULL(elem);

  // 9) check NESTED_ARRAY: 2×2 matrix [[0,1],[10,11]]
  struct cmc_ConfigField *row = f_nested_array->value;
  int out_i;
  // row 0
  struct cmc_ConfigField *cell = row->value;
  err = cmc_field_get_int(cell, &out_i);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(0, out_i);
  cell = cell->next_field;
  err = cmc_field_get_int(cell, &out_i);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(1, out_i);
  // row 1
  row = row->next_field;
  TEST_ASSERT_NOT_NULL(row);
  cell = row->value;
  err = cmc_field_get_int(cell, &out_i);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(10, out_i);
  cell = cell->next_field;
  err = cmc_field_get_int(cell, &out_i);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(11, out_i);

  // 10) check DOUBLE_NESTED_ARRAY:
  // [ [ [0,1,2], [10,11] ], [ [100], [110], [120] ] ]
  struct cmc_ConfigField *outer = f_double_array->value;
  int expected0[][3] = {{0, 1, 2}, {10, 11, -1}}; // -1 filler ignored
  int expected1[][1] = {{100}, {110}, {120}};
  // outer[0]
  struct cmc_ConfigField *mid = outer->value;
  // mid[0] → leafs: 0,1,2
  struct cmc_ConfigField *leaf = mid->value;
  for (int k = 0; k < 3; k++) {
    TEST_ASSERT_NOT_NULL(leaf);
    err = cmc_field_get_int(leaf, &out_i);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_INT(expected0[0][k], out_i);
    leaf = leaf->next_field;
  }
  // mid[1] → leafs: 10,11
  mid = mid->next_field;
  TEST_ASSERT_NOT_NULL(mid);
  leaf = mid->value;
  for (int k = 0; k < 2; k++) {
    TEST_ASSERT_NOT_NULL(leaf);
    err = cmc_field_get_int(leaf, &out_i);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_INT(expected0[1][k], out_i);
    leaf = leaf->next_field;
  }
  // outer[1]
  outer = outer->next_field;
  TEST_ASSERT_NOT_NULL(outer);
  mid = outer->value;
  for (int j = 0; j < 3; j++) {
    TEST_ASSERT_NOT_NULL(mid);
    leaf = mid->value;
    err = cmc_field_get_int(leaf, &out_i);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_INT(expected1[j][0], out_i);
    mid = mid->next_field;
  }
}
