#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unity.h>

#include "c_minilib_config.h"
#include "c_minilib_error.h"
#include "utils/cmc_error.h"
#include "utils/cmc_field.h"
#include "utils/cmc_settings.h"

#ifndef CONFIG_DIR
#error "CONFIG_DIR must be defined to point at the base directory for superApp.env"
#endif

static struct cmc_Config *config = NULL;
static cmc_error_t err = NULL;

void setUp(void) {
    config = NULL;
    err = NULL;
}

void tearDown(void) {
    cmc_config_destroy(&config);
    cmc_error_destroy(&err);
}

void test_cmc_config_create_null_output(void) {
    err = cmc_config_create(NULL, NULL);
    TEST_ASSERT_NOT_NULL(err);
}

void test_cmc_config_create_with_null_settings(void) {
    err = cmc_config_create(NULL, &config);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_NOT_NULL(config->settings);
}

void test_cmc_config_create_with_valid_settings(void) {
    char *paths[] = {"/path/one", "/path/two"};

    err = cmc_config_create(&(struct cmc_ConfigSettings){
        .supported_paths = paths,
        .paths_length = 2,
        .name = "test_config",
        .log_func = NULL
    }, &config);

    TEST_ASSERT_NULL(err);
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_NOT_NULL(config->settings);
    TEST_ASSERT_EQUAL_UINT32(3, config->settings->paths_length);
    TEST_ASSERT_EQUAL_STRING("test_config", config->settings->name);
}

void test_cmc_config_add_field_null_args(void) {
    err = cmc_config_add_field(NULL, NULL);
    TEST_ASSERT_NOT_NULL(err);
}

#define FIELD_NAME_MAX 32
#define TEST_LOOPS_MAX 10
void test_cmc_config_add_valid_field(void) {
    err = cmc_config_create(NULL, &config);
    TEST_ASSERT_NULL(err);

    for (int i = 0; i < TEST_LOOPS_MAX; i++) {
        char name[FIELD_NAME_MAX];
        sprintf(name, "server_host_%d", i);

        struct cmc_ConfigField *field;
        err = cmc_field_create(name, cmc_ConfigFieldTypeEnum_STRING, "localhost", true, &field);
        TEST_ASSERT_NULL(err);

        err = cmc_config_add_field(field, config);
        TEST_ASSERT_NULL(err);
        TEST_ASSERT_NOT_NULL(config->fields);
        TEST_ASSERT_EQUAL_STRING(name, config->fields->name);
        TEST_ASSERT_EQUAL_STRING("localhost", (char *)config->fields->default_value);
        TEST_ASSERT_TRUE(config->fields->optional);
    }
}

void test_cmc_lib_init_idempotent(void) {
    err = cmc_lib_init();
    TEST_ASSERT_NULL(err);
    err = cmc_lib_init();
    TEST_ASSERT_NULL(err);
}

void test_parse_superApp_env_and_getters(void) {
    err = cmc_lib_init();
    TEST_ASSERT_NULL(err);

    char *paths[] = {(char *)CONFIG_DIR};
    err = cmc_config_create(&(struct cmc_ConfigSettings){
        .supported_paths = paths,
        .paths_length = 1,
        .name = "superApp",
        .log_func = NULL
    }, &config);
    TEST_ASSERT_NULL(err);

    struct cmc_ConfigField *f_name, *f_amount, *f_something;
    static int def_amount = 0;

    err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, "<none>", true, &f_name);
    TEST_ASSERT_NULL(err);
    err = cmc_config_add_field(f_name, config);
    TEST_ASSERT_NULL(err);

    err = cmc_field_create("amount", cmc_ConfigFieldTypeEnum_INT, &def_amount, true, &f_amount);
    TEST_ASSERT_NULL(err);
    err = cmc_config_add_field(f_amount, config);
    TEST_ASSERT_NULL(err);

    err = cmc_field_create("something", cmc_ConfigFieldTypeEnum_STRING, "<empty>", true, &f_something);
    TEST_ASSERT_NULL(err);
    err = cmc_config_add_field(f_something, config);
    TEST_ASSERT_NULL(err);

    err = cmc_config_parse(config);
    TEST_ASSERT_NULL(err);

    char *val_name = NULL;
    err = cmc_field_get_str(f_name, &val_name);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_STRING("whatever", val_name);

    int val_amount = -1;
    err = cmc_field_get_int(f_amount, &val_amount);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_INT(1, val_amount);

    char *val_something = NULL;
    err = cmc_field_get_str(f_something, &val_something);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_STRING("<empty>", val_something);
}

void test_getters_on_unset_required_field_str(void) {
    err = cmc_config_create(NULL, &config);
    TEST_ASSERT_NULL(err);

    struct cmc_ConfigField *field;
    err = cmc_field_create("required_str", cmc_ConfigFieldTypeEnum_STRING, NULL, false, &field);
    TEST_ASSERT_NULL(err);

    err = cmc_config_add_field(field, config);
    TEST_ASSERT_NULL(err);

    char *out_s = NULL;
    err = cmc_field_get_str(field, &out_s);
    TEST_ASSERT_NOT_NULL(err);
}

void test_getters_on_unset_required_field_int(void) {
    err = cmc_config_create(NULL, &config);
    TEST_ASSERT_NULL(err);

    struct cmc_ConfigField *field;
    err = cmc_field_create("required_int", cmc_ConfigFieldTypeEnum_INT, NULL, false, &field);
    TEST_ASSERT_NULL(err);

    err = cmc_config_add_field(field, config);
    TEST_ASSERT_NULL(err);

    int out_i = -1;
    err = cmc_field_get_int(field, &out_i);
    TEST_ASSERT_NOT_NULL(err);
}
