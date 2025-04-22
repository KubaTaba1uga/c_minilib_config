
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
void __test_optional_array_parsing(void) {
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

void __test_double_nested_array_parsing(void) {
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

void __test_optional_field_from_dict_env(void) {
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

void __test_optional_dict_fields_dict_name_and_dict_age(void) {
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

void __test_nested_dict_person_fields(void) {
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

  // nested_dict (outer)
  err = cmc_field_create("nested_dict", cmc_ConfigFieldTypeEnum_DICT, NULL,
                         true, &f_outer_dict);
  TEST_ASSERT_NULL(err);

  // person (inner dict)
  err = cmc_field_create("person", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &f_inner_dict);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_outer_dict, f_inner_dict);
  TEST_ASSERT_NULL(err);

  // person.name (optional)
  err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, "<none>", true,
                         &f_name);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_inner_dict, f_name);
  TEST_ASSERT_NULL(err);

  // person.age (optional)
  err =
      cmc_field_create("age", cmc_ConfigFieldTypeEnum_INT, NULL, true, &f_age);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_next_field(f_name, f_age);
  TEST_ASSERT_NULL(err);

  // person.optional (optional, not present)
  err = cmc_field_create("optional", cmc_ConfigFieldTypeEnum_STRING, "n/a",
                         true, &f_optional);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_next_field(f_age, f_optional);
  TEST_ASSERT_NULL(err);

  err = cmc_config_add_field(f_outer_dict, config);
  TEST_ASSERT_NULL(err);

  err = parser.parse(strlen(DICT_CONFIG_PATH), DICT_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  // Validate values
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

void __test_parse_list_of_persons_env_file(void) {
  // 1) init parser & config
  err = cmc_env_parser_init(&parser);
  TEST_ASSERT_NULL(err);

  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)DICT_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "list",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  // 2) declare `list` as ARRAY → nested `person` DICT → fields `name`, `age`
  struct cmc_ConfigField *f_list, *f_dict, *f_person, *f_name, *f_age;

  err = cmc_field_create("list", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &f_list);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(f_list, config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &f_dict);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_list, f_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("person", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &f_person);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_dict, f_person);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &f_name);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(f_person, f_name);
  TEST_ASSERT_NULL(err);

  err =
      cmc_field_create("age", cmc_ConfigFieldTypeEnum_INT, NULL, true, &f_age);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_next_field(f_name, f_age);
  TEST_ASSERT_NULL(err);

  // 3) parse via the ENV‐parser
  err = parser.parse(strlen(DICT_CONFIG_PATH), DICT_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  // 4) check first element
  char *out_s = NULL;
  int out_i = -1;

  err = cmc_field_get_str(f_name, &out_s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("john", out_s);

  err = cmc_field_get_int(f_age, &out_i);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(99, out_i);

  // 5) advance to second array element
  //    deep‑clone should have produced f_list->value->next_field

  TEST_ASSERT_NOT_NULL(f_dict->next_field);
  if (!f_dict->next_field) {
    return;
  }
  struct cmc_ConfigField *second_dict = f_dict->next_field;
  TEST_ASSERT_NOT_NULL(second_dict);
  struct cmc_ConfigField *second_person = second_dict->value;
  TEST_ASSERT_NOT_NULL(second_person);

  //    under that, the `name` and `age` clones live exactly
  //    as `second_person->value` and its ->next_field
  struct cmc_ConfigField *second_name = second_person->value;
  struct cmc_ConfigField *second_age = second_name->next_field;

  err = cmc_field_get_str(second_name, &out_s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("alan", out_s);

  err = cmc_field_get_int(second_age, &out_i);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(47, out_i);
}

void __test_kea_interfaces_config_parsing(void) {
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *root, *cfg, *arr, *elem;
  if (!err)
    err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                           &root);
  if (!err)
    err = cmc_config_add_field(root, config);
  if (!err)
    err = cmc_field_create("interfaces_config", cmc_ConfigFieldTypeEnum_DICT,
                           NULL, true, &cfg);
  if (!err)
    err = cmc_field_add_nested_field(root, cfg);
  if (!err)
    err = cmc_field_create("interfaces", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                           true, &arr);
  if (!err)
    err = cmc_field_add_nested_field(cfg, arr);
  if (!err)
    err =
        cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &elem);
  if (!err)
    err = cmc_field_add_nested_field(arr, elem);
  if (!err)
    err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);

  TEST_ASSERT_NULL(err);
  char *out = NULL;
  err = cmc_field_get_str(elem, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("eth0", out);
}

void __test_kea_lease_database_type_parsing(void) {
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *root, *lease, *type;
  if (!err)
    err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                           &root);
  if (!err)
    err = cmc_config_add_field(root, config);
  if (!err)
    err = cmc_field_create("lease_database", cmc_ConfigFieldTypeEnum_DICT, NULL,
                           true, &lease);
  if (!err)
    err = cmc_field_add_nested_field(root, lease);
  if (!err)
    err = cmc_field_create("type", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                           &type);
  if (!err)
    err = cmc_field_add_nested_field(lease, type);
  if (!err)
    err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);

  TEST_ASSERT_NULL(err);
  char *out = NULL;
  err = cmc_field_get_str(type, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("memfile", out);
}

void __test_kea_lease_database_name_parsing(void) {
  struct cmc_ConfigField *root, *lease, *name;
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  if (!err)
    err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                           &root);
  if (!err)
    err = cmc_config_add_field(root, config);
  if (!err)
    err = cmc_field_create("lease_database", cmc_ConfigFieldTypeEnum_DICT, NULL,
                           true, &lease);
  if (!err)
    err = cmc_field_add_nested_field(root, lease);
  if (!err)
    err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                           &name);
  if (!err)
    err = cmc_field_add_nested_field(lease, name);
  if (!err)
    err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);

  TEST_ASSERT_NULL(err);
  char *out = NULL;
  err = cmc_field_get_str(name, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("/var/lib/kea/dhcp4.leases", out);
}

void __test_kea_subnet_pool_value_parsing(void) {
  struct cmc_ConfigField *root, *subnet_arr, *subnet, *pools, *pool_dict,
      *pool_val;
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  if (!err)
    err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                           &root);
  if (!err)
    err = cmc_config_add_field(root, config);
  if (!err)
    err = cmc_field_create("subnet4", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                           &subnet_arr);
  if (!err)
    err = cmc_field_add_nested_field(root, subnet_arr);
  if (!err)
    err =
        cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &subnet);
  if (!err)
    err = cmc_field_add_nested_field(subnet_arr, subnet);
  if (!err)
    err = cmc_field_create("pools", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                           &pools);
  if (!err)
    err = cmc_field_add_nested_field(subnet, pools);
  if (!err)
    err = cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                           &pool_dict);
  if (!err)
    err = cmc_field_add_nested_field(pools, pool_dict);
  if (!err)
    err = cmc_field_create("pool", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                           &pool_val);
  if (!err)
    err = cmc_field_add_nested_field(pool_dict, pool_val);
  if (!err)
    err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);

  TEST_ASSERT_NULL(err);
  char *out = NULL;
  err = cmc_field_get_str(pool_val, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("192.168.1.100 - 192.168.1.200", out);
}

void __test_kea_option_data_second_value_parsing(void) {
  struct cmc_ConfigField *dhcp4, *subnet4, *subnet4_dict, *opt_data, *opt_dict,
      *data, *name;

  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  // Define top dict and array
  err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &dhcp4);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(dhcp4, config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("subnet4", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &subnet4);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(dhcp4, subnet4);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &subnet4_dict);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(subnet4, subnet4_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("option_data", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &opt_data);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(subnet4_dict, opt_data);
  TEST_ASSERT_NULL(err);

  err =
      cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &opt_dict);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(opt_data, opt_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &name);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_nested_field(opt_dict, name);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("data", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &data);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_next_field(name, data);
  TEST_ASSERT_NULL(err);

  // Parse
  err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *opt1 = opt_dict->next_field;
  TEST_ASSERT_NOT_NULL(opt1);

  struct cmc_ConfigField *opt1_name = (struct cmc_ConfigField *)opt1->value;
  TEST_ASSERT_NOT_NULL(opt1_name);
  struct cmc_ConfigField *opt1_data = opt1_name->next_field;
  TEST_ASSERT_NOT_NULL(opt1_data);
  TEST_ASSERT_NOT_NULL(opt1_data->value);

  char *out = NULL;
  err = cmc_field_get_str(opt1_data, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("8.8.8.8", out);
}

void __test_parse_full_kea_env_file(void) {
  err = cmc_env_parser_init(&parser);
  TEST_ASSERT_NULL(err);

  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *dhcp4, *ifcfg, *ifaces, *iface0;
  struct cmc_ConfigField *lease, *type, *persist, *name;
  struct cmc_ConfigField *subnets, *subnet, *subnet_val;
  struct cmc_ConfigField *pools, *pool_dict, *pool_val;
  struct cmc_ConfigField *opts, *opt0, *opt0_name, *opt0_data;
  struct cmc_ConfigField *opt1, *opt1_name, *opt1_data;
  struct cmc_ConfigField *lifetime;

  // Build config field hierarchy
  cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &dhcp4);
  cmc_config_add_field(dhcp4, config);

  cmc_field_create("interfaces_config", cmc_ConfigFieldTypeEnum_DICT, NULL,
                   true, &ifcfg);
  cmc_field_add_nested_field(dhcp4, ifcfg);

  cmc_field_create("interfaces", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                   &ifaces);
  cmc_field_add_nested_field(ifcfg, ifaces);

  cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &iface0);
  cmc_field_add_nested_field(ifaces, iface0);

  cmc_field_create("lease_database", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                   &lease);
  cmc_field_add_next_field(ifcfg, lease);

  cmc_field_create("type", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &type);
  cmc_field_add_nested_field(lease, type);

  cmc_field_create("persist", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &persist);
  cmc_field_add_next_field(type, persist);

  cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &name);
  cmc_field_add_next_field(persist, name);

  cmc_field_create("subnet4", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                   &subnets);
  cmc_field_add_next_field(lease, subnets);

  cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &subnet);
  cmc_field_add_nested_field(subnets, subnet);

  cmc_field_create("subnet", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &subnet_val);
  cmc_field_add_nested_field(subnet, subnet_val);

  cmc_field_create("pools", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true, &pools);
  cmc_field_add_next_field(subnet_val, pools);

  cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &pool_dict);
  cmc_field_add_nested_field(pools, pool_dict);

  cmc_field_create("pool", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &pool_val);
  cmc_field_add_nested_field(pool_dict, pool_val);

  cmc_field_create("option_data", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                   &opts);
  cmc_field_add_next_field(pools, opts);

  cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &opt0);
  cmc_field_add_nested_field(opts, opt0);

  cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &opt0_name);
  cmc_field_add_nested_field(opt0, opt0_name);

  cmc_field_create("data", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &opt0_data);
  cmc_field_add_next_field(opt0_name, opt0_data);

  cmc_field_create("valid_lifetime", cmc_ConfigFieldTypeEnum_INT, NULL, true,
                   &lifetime);
  cmc_field_add_next_field(subnets, lifetime);

  // Parse using the ENV parser directly
  err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  char *s = NULL;
  int v = -1;

  cmc_field_get_str(iface0, &s);
  TEST_ASSERT_EQUAL_STRING("eth0", s);

  cmc_field_get_str(type, &s);
  TEST_ASSERT_EQUAL_STRING("memfile", s);

  cmc_field_get_str(persist, &s);
  TEST_ASSERT_EQUAL_STRING("true", s);

  cmc_field_get_str(name, &s);
  TEST_ASSERT_EQUAL_STRING("/var/lib/kea/dhcp4.leases", s);

  cmc_field_get_str(subnet_val, &s);
  TEST_ASSERT_EQUAL_STRING("192.168.1.0/24", s);

  cmc_field_get_str(pool_val, &s);
  TEST_ASSERT_EQUAL_STRING("192.168.1.100 - 192.168.1.200", s);

  cmc_field_get_str(opt0_name, &s);
  TEST_ASSERT_EQUAL_STRING("routers", s);
  cmc_field_get_str(opt0_data, &s);
  TEST_ASSERT_EQUAL_STRING("192.168.1.1", s);

  opt1 = opt0->next_field;
  TEST_ASSERT_NOT_NULL(opt1);
  opt1_name = opt1->value;
  TEST_ASSERT_NOT_NULL(opt1_name);
  opt1_data = opt1_name->next_field;
  TEST_ASSERT_NOT_NULL(opt1_data);

  cmc_field_get_str(opt1_name, &s);
  TEST_ASSERT_EQUAL_STRING("domain-name-servers", s);
  cmc_field_get_str(opt1_data, &s);
  TEST_ASSERT_EQUAL_STRING("8.8.8.8", s);

  cmc_field_get_int(lifetime, &v);
  TEST_ASSERT_EQUAL_INT(3600, v);
}
