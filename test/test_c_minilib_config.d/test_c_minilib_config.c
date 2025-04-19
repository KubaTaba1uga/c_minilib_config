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

// Basic creation tests
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

    err = cmc_config_create(
        &(struct cmc_ConfigSettings){
            .supported_paths = paths,
            .paths_length = 2,
            .name = "test_config",
            .log_func = NULL
        },
        &config
    );

    TEST_ASSERT_NULL(err);
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_NOT_NULL(config->settings);
    TEST_ASSERT_EQUAL_UINT32(2, config->settings->paths_length);
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

        err = cmc_config_add_field(
            &(struct cmc_ConfigField){
                .name = name,
                .type = cmc_ConfigFieldTypeEnum_STRING,
                .default_value = "localhost",
                .optional = true
            },
            config
        );

        TEST_ASSERT_NULL(err);
        TEST_ASSERT_NOT_NULL(config->fields);
        TEST_ASSERT_EQUAL_STRING(name, config->fields->name);
        TEST_ASSERT_EQUAL_STRING("localhost", config->fields->default_value);
        TEST_ASSERT_TRUE(config->fields->optional);
    }
}

// Extended tests: initialization, parsing superApp.env, getters
void test_cmc_lib_init_idempotent(void) {
    err = cmc_lib_init();
    TEST_ASSERT_NULL(err);
    err = cmc_lib_init();
    TEST_ASSERT_NULL(err);
}

void test_parse_superApp_env_and_getters(void) {
    /* Initialize parser */
    err = cmc_lib_init();
    TEST_ASSERT_NULL(err);

    /* Point at superApp.env */
    char *paths[] = { (char *)CONFIG_DIR };
    err = cmc_config_create(
        &(struct cmc_ConfigSettings){
            .supported_paths = paths,
            .paths_length = 1,
            .name = "superApp",
            .log_func = NULL
        },
        &config
    );
    TEST_ASSERT_NULL(err);

    /* Define expected fields */
    static int def_amount = 0;
    err = cmc_config_add_field(
        &(struct cmc_ConfigField){
            .name = "name",
            .type = cmc_ConfigFieldTypeEnum_STRING,
            .default_value = "<none>",
            .optional = true
        },
        config
    );
    TEST_ASSERT_NULL(err);
    err = cmc_config_add_field(
        &(struct cmc_ConfigField){
            .name = "amount",
            .type = cmc_ConfigFieldTypeEnum_INT,
            .default_value = &def_amount,
            .optional = true
        },
        config
    );
    TEST_ASSERT_NULL(err);
    err = cmc_config_add_field(
        &(struct cmc_ConfigField){
            .name = "something",
            .type = cmc_ConfigFieldTypeEnum_STRING,
            .default_value = "<empty>",
            .optional = true
        },
        config
    );
    TEST_ASSERT_NULL(err);

    /* Parse */
    err = cmc_config_parse(config);
    TEST_ASSERT_NULL(err);

    /* Validate NAME -> "whatever" */
    char *s_name = NULL;
    err = cmc_config_get_str("name", config, &s_name);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_STRING("whatever", s_name);

    /* Validate AMOUNT -> 1 */
    int v_amount = -1;
    err = cmc_config_get_int("amount", config, &v_amount);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_INT(1, v_amount);

    /* Validate SOMETHING -> default "<empty>" */
    char *s_som = NULL;
    err = cmc_config_get_str("something", config, &s_som);
    TEST_ASSERT_NULL(err);
    TEST_ASSERT_EQUAL_STRING("<empty>", s_som);
}

void test_getters_on_missing_field_return_error_str(void) {
    err = cmc_lib_init();
    TEST_ASSERT_NULL(err);
    err = cmc_config_create(NULL, &config);
    TEST_ASSERT_NULL(err);

    char *out_s;
    err = cmc_config_get_str("no_such", config, &out_s);
    TEST_ASSERT_NOT_NULL(err);

}

void test_getters_on_missing_field_return_error_int(void) {
    err = cmc_lib_init();
    TEST_ASSERT_NULL(err);
    err = cmc_config_create(NULL, &config);
    TEST_ASSERT_NULL(err);

    int out_i;
    err = cmc_config_get_int("no_such", config, &out_i);
    TEST_ASSERT_NOT_NULL(err);
}

