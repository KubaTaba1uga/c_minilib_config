#include "c_minilib_error.h"
#include <c_minilib_config.h>
#include <stdio.h>
#include <stdlib.h>

void log_(enum cmc_LogLevelEnum _, char *msg) { puts(msg); }

int main(void) {
  cmc_error_t err;

  // 1. Initialize library
  err = cmc_lib_init();
  if (err) {
    fprintf(stderr, "init error\n");
    return 1;
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
  if (err)
    return 2;

  // 3. Define fields
  struct cmc_ConfigField *f_name, *f_port, *f_users, *f_user_elem, *f_meta,
      *f_version, *f_license;

  err = cmc_field_create("app_name", cmc_ConfigFieldTypeEnum_STRING,
                         "default_app", true, &f_name);
  cmc_config_add_field(f_name, config);

  int default_port = 80;
  err = cmc_field_create("app_port", cmc_ConfigFieldTypeEnum_INT, &default_port,
                         true, &f_port);
  cmc_config_add_field(f_port, config);

  err = cmc_field_create("app_users", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &f_users);
  cmc_config_add_field(f_users, config);
  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &f_user_elem);
  cmc_field_add_subfield(f_users, f_user_elem);

  err = cmc_field_create("app_meta", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &f_meta);
  cmc_config_add_field(f_meta, config);

  err = cmc_field_create("version", cmc_ConfigFieldTypeEnum_STRING, "0.0.0",
                         true, &f_version);
  cmc_field_add_subfield(f_meta, f_version);

  err = cmc_field_create("license", cmc_ConfigFieldTypeEnum_STRING, "unknown",
                         true, &f_license);
  cmc_field_add_subfield(f_meta, f_license);

  // 4. Parse the config
  err = cmc_config_parse(config);
  if (err) {
    fprintf(stderr, "parse error\n");
    cmc_config_destroy(&config);
    cme_error_dump(err, "error_dump.txt");
    cme_error_destroy(err);
    return 3;
  }

  // 5. Retrieve values
  char *name = NULL, *version = NULL, *license = NULL;
  int port = -1;

  cmc_field_get_str(f_name, &name);
  cmc_field_get_int(f_port, &port);
  cmc_field_get_str(f_version, &version);
  cmc_field_get_str(f_license, &license);

  printf("%12s: %s\n", "App Name", name);
  printf("%12s: %d\n", "App Port", port);
  printf("%12s: %s\n", "Meta Version", version);
  printf("%12s: %s\n", "Meta License", license);

  printf("%12s:\n", "Users");
  CMC_FOREACH_FIELD_ARRAY(user, char *, &f_users,
                          { printf("%14s %s\n", "-", user); });

  // 6. Cleanup
  cmc_config_destroy(&config);
  cmc_lib_destroy();
  return 0;
}
