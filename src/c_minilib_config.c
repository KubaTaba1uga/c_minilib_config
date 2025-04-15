#include <asm-generic/errno.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "c_minilib_config.h"
#include "c_minilib_error.h"
#include "cmc_common.h"
#include "cmc_env_parser/cmc_env_parser.h"
#include "cmc_error.h"
#include "cmc_parse_interface.h"

static struct cmc_ConfigParseInterface parsers[cmc_ConfigParseFormat_MAX];
static int32_t parsers_length = 0;

cmc_error_t cmc_lib_init(void) {
  cmc_error_t err;

  if (parsers_length != 0) {
    return NULL;
  }

  err = cmc_env_parser_init(&parsers[parsers_length++]);
  if (err) {
    return err;
  }

  return NULL;
};

cmc_error_t cmc_config_create(const struct cmc_ConfigSettings *settings,
                              struct cmc_Config **config) {
  struct cmc_Config *local_config;
  cmc_error_t err;

  if (!settings || !config) {
    err = cmc_errorf(EINVAL, "`settings=%p` and `config=%p` cannot be NULL\n",
                     settings, config);
    goto error_out;
  }

  if (settings->paths_length > CMC_SETTINGS_ARR_PATHS_MAX) {
    err = cmc_errorf(EINVAL, "`settings->paths_length=%d` too big. Max is %d\n",
                     settings->paths_length, CMC_SETTINGS_ARR_PATHS_MAX);
    goto error_out;
  }

  local_config = malloc(sizeof(struct cmc_Config));
  if (!local_config) {
    err = cmc_errorf(ENOMEM, "Unable to allocate memory for `local_config`\n");
    goto error_out;
  }

  local_config->settings = malloc(sizeof(struct cmc_ConfigSettings));
  if (!local_config->settings) {
    err = cmc_errorf(
        ENOMEM, "Unable to allocate memory for `local_config->settings`\n");
    goto error_config_cleanup;
  }

  local_config->settings->name = calloc(CMC_SETTINGS_NAME_MAX, sizeof(char));
  if (!local_config->settings->name) {
    err = cmc_errorf(
        ENOMEM,
        "Unable to allocate memory for `local_config->settings->name`\n");
    goto error_settings_cleanup;
  }

  if (!settings->name) {
    strncpy("config", settings->name, CMC_SETTINGS_NAME_MAX - 1);
  } else {
    strncpy(local_config->settings->name, settings->name,
            CMC_SETTINGS_NAME_MAX - 1);
  };

  local_config->settings->supported_paths =
      calloc(CMC_SETTINGS_ARR_PATHS_MAX, CMC_SETTINGS_PATHS_MAX * sizeof(char));
  if (!local_config->settings->supported_paths) {
    err = cmc_errorf(ENOMEM, "Unable to allocate memory for "
                             "`local_config->settings->supported_paths`\n");
    goto error_settings_name_cleanup;
  }

  local_config->settings->paths_length = 0;

  if (getcwd(local_config->settings
                 ->supported_paths[local_config->settings->paths_length++],
             CMC_SETTINGS_PATHS_MAX) == NULL) {
    err = cmc_errorf(ENOMEM, "Unable to get current working directory`\n");
    goto error_settings_paths_cleanup;
  }

  for (int i = 0; i < local_config->settings->paths_length; i++) {
    strncpy(local_config->settings
                ->supported_paths[local_config->settings->paths_length++],
            settings->supported_paths[i], CMC_SETTINGS_PATHS_MAX);
  }

  local_config->fields = NULL;

  *config = local_config;

  return NULL;

error_settings_paths_cleanup:
  free(local_config->settings->supported_paths);
error_settings_name_cleanup:
  free(local_config->settings->name);
error_settings_cleanup:
  free(local_config->settings);
error_config_cleanup:
  free(local_config);
error_our:
  return err;
};

void cmc_config_destroy(struct cmc_Config **config) {
  if (!config || !*config) {
    return;
  }

  free(*config->settings->supported_paths);
  free(*config->settings->name);
  free(*config->settings);
  free(*config);
  *config = NULL;
};

cmc_error_t cmc_config_add_field(const struct cmc_ConfigField *field,
                                 struct cmc_Config *config);
cmc_error_t cmc_config_parse(struct cmc_Config *config);
cmc_error_t cmc_config_destroy(struct cmc_Config *config);
cmc_error_t cmc_config_get_str(const char *name,
                               const struct cmc_Config *config, size_t n,
                               char buffer[n]);
cmc_error_t cmc_config_get_int(const char *name,
                               const struct cmc_Config *config, int *output);
void cmc_error_destroy(cmc_error_t *error);
