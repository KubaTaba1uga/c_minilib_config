#include <asm-generic/errno.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "c_minilib_config.h"
#include "c_minilib_error.h"
#include "cmc_parse_interface/cmc_env_parser/cmc_env_parser.h"
#include "cmc_parse_interface/cmc_parse_interface.h"
#include "utils/cmc_common.h"
#include "utils/cmc_error.h"
#include "utils/cmc_settings.h"

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

  if (!config) {
    err = cmc_errorf(EINVAL, "`config=%p` cannot be NULL\n", settings, config);
    goto error_out;
  }

  local_config = malloc(sizeof(struct cmc_Config));
  if (!local_config) {
    err = cmc_errorf(ENOMEM, "Unable to allocate memory for `local_config`\n");
    goto error_out;
  }

  if (!settings) {
    err = cmc_settings_create(0, NULL, NULL, NULL, &local_config->settings);
    if (err) {
      goto error_config_cleanup;
    }
  }

  local_config->settings = (struct cmc_ConfigSettings *)settings;
  local_config->fields = NULL;

  *config = local_config;

  return NULL;

error_config_cleanup:
  free(local_config);
error_out:
  return err;
};

void cmc_config_destroy(struct cmc_Config **config) {
  if (!config || !*config) {
    return;
  }

  cmc_settings_destroy(&(*config)->settings);
  free(*config);

  *config = NULL;
};

/* cmc_error_t cmc_config_add_field(const struct cmc_ConfigField *field, */
/*                                  struct cmc_Config *config); */
/* cmc_error_t cmc_config_parse(struct cmc_Config *config); */
/* cmc_error_t cmc_config_get_str(const char *name, */
/*                                const struct cmc_Config *config, size_t n, */
/*                                char buffer[n]); */
/* cmc_error_t cmc_config_get_int(const char *name, */
/*                                const struct cmc_Config *config, int *output);
 */
