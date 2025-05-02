#include "c_minilib_error.h"
#include <c_minilib_config.h>
#include <stdio.h>
#include <stdlib.h>

void log_(enum cmc_LogLevelEnum _, char *msg) { puts(msg); }

int main(void) {
  cme_error_t err = NULL;

  // 1. Initialize library
  err = cmc_lib_init();
  if (err) {
    goto error_out;
  }

  // 2. Create config context
  struct cmc_Config *config = NULL;
  char *paths[] = {".", "./example"};
  err = cmc_config_create(
      &(struct cmc_ConfigSettings){
          .supported_paths = paths,
          .paths_length = 2,
          .name = "example", // Will look for ./example.env
          .log_func = log_,
      },
      &config);
  if (err) {
    goto error_out;
  }

  // 3. Define fields
  struct cmc_ConfigField *f_name, *f_port, *f_users, *f_user_elem, *f_meta,
      *f_version, *f_license;

  err = cmc_field_create("app_name", cmc_ConfigFieldTypeEnum_STRING,
                         "default_app", true, &f_name);
  if (err) {
    goto error_out;
  }
  err = cmc_config_add_field(f_name, config);
  if (err) {
    goto error_out;
  }

  int default_port = 80;
  err = cmc_field_create("app_port", cmc_ConfigFieldTypeEnum_INT, &default_port,
                         true, &f_port);
  if (err) {
    goto error_out;
  }
  err = cmc_config_add_field(f_port, config);
  if (err) {
    goto error_out;
  }

  err = cmc_field_create("app_users", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &f_users);
  if (err) {
    goto error_out;
  }
  err = cmc_config_add_field(f_users, config);
  if (err) {
    goto error_out;
  }

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &f_user_elem);
  if (err) {
    goto error_out;
  }
  err = cmc_field_add_subfield(f_users, f_user_elem);
  if (err) {
    goto error_out;
  }

  err = cmc_field_create("app_meta", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &f_meta);
  if (err) {
    goto error_out;
  }
  err = cmc_config_add_field(f_meta, config);
  if (err) {
    goto error_out;
  }

  err = cmc_field_create("version", cmc_ConfigFieldTypeEnum_STRING, "0.0.0",
                         true, &f_version);
  if (err) {
    goto error_out;
  }
  err = cmc_field_add_subfield(f_meta, f_version);
  if (err) {
    goto error_out;
  }

  err = cmc_field_create("license", cmc_ConfigFieldTypeEnum_STRING, "unknown",
                         true, &f_license);
  if (err) {
    goto error_out;
  }
  err = cmc_field_add_subfield(f_meta, f_license);
  if (err) {
    goto error_out;
  }

  // 4. Parse the config
  err = cmc_config_parse(config);
  if (err) {
    goto error_out;
  }

  // 5. Retrieve values
  char *name = NULL, *version = NULL, *license = NULL;
  int port = -1;

  err = cmc_field_get_str(f_name, &name);
  if (err) {
    goto error_out;
  }
  err = cmc_field_get_int(f_port, &port);
  if (err) {
    goto error_out;
  }
  err = cmc_field_get_str(f_version, &version);
  if (err) {
    goto error_out;
  }
  err = cmc_field_get_str(f_license, &license);
  if (err) {
    goto error_out;
  }

  printf("%12s: %s\n", "App Name", name);
  printf("%12s: %d\n", "App Port", port);
  printf("%12s: %s\n", "Meta Version", version);
  printf("%12s: %s\n", "Meta License", license);

  printf("%12s:\n", "Users");
  CMC_FOREACH_FIELD_ARRAY(user, char *, &f_users,
                          { printf("%14s %s\n", "-", user); });

  cmc_config_destroy(&config);
  cmc_lib_destroy();
  return 0;

error_out:
  fprintf(stderr, "Error occurred:\n");
  if (err) {
    cme_error_dump_to_file(err, "error_dump.txt");
    cme_error_destroy(err);
  }
  cmc_config_destroy(&config);
  cmc_lib_destroy();
  return 1;
}
