#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "c_minilib_config.h"
#include "utils/cmc_error.h"

cmc_error_t cmc_settings_create(const uint32_t paths_length,
                                const char **supported_paths, const char *name,
                                const void *log_func,
                                struct cmc_ConfigSettings **settings) {
  struct cmc_ConfigSettings *local_settings;
  cmc_error_t err;

  if (!settings) {
    err = cmc_errorf(EINVAL, "`settings=%p` cannot be NULL\n", settings);
    goto error_out;
  }

  local_settings = malloc(sizeof(struct cmc_ConfigSettings));
  if (!local_settings) {
    err =
        cmc_errorf(ENOMEM, "Unable to allocate memory for `local_settings`\n");
    goto error_out;
  }

  uint32_t local_paths_length = paths_length;

  if (!supported_paths) {
    local_paths_length = 0;
  }

  local_settings->paths_length = 0;
  local_settings->supported_paths = NULL;

  for (uint32_t i = 0, additional_paths = 1;
       i < local_paths_length + additional_paths; i++) {
    char **local_supported_paths;

    local_supported_paths = (char **)realloc(
        (void *)local_settings->supported_paths, sizeof(char *) * (i + 1));
    if (!local_supported_paths) {
      err = cmc_errorf(ENOMEM,
                       "Unable to allocate memory for `local_supported_paths`, "
                       "requested memory size: %d\n",
                       sizeof(char *) * (i + 1));
      goto error_settings_paths_cleanup;
    }

    local_settings->supported_paths = local_supported_paths;

    // First we are injecting additional paths
    if (i == 0) {
      local_settings->supported_paths[i] = getcwd(NULL, 0);
    } else {
      // Here starts normal processing
      local_settings->supported_paths[i] =
          strdup(supported_paths[i - additional_paths]);
    }

    if (!local_settings->supported_paths) {
      err = cmc_errorf(ENOMEM,
                       "Unable to allocate memory for "
                       "`local_settings->supported_paths[%d]`\n",
                       i);
      goto error_settings_paths_iter_cleanup;
    }

    // We need to add 1 for length
    local_settings->paths_length = i + 1;
  }

  if (!name) {
    name = "config";
  }

  local_settings->name = strdup(name);
  if (!local_settings->name) {
    err = cmc_errorf(ENOMEM, "Unable to allocate memory for "
                             "`local_settings->name`\n");
    goto error_settings_paths_iter_cleanup;
  }

  local_settings->log_func = log_func;

  *settings = local_settings;

  return NULL;

error_settings_paths_iter_cleanup:
  while (local_settings->paths_length-- > 0) {
    free(local_settings->supported_paths[local_settings->paths_length]);
  }
error_settings_paths_cleanup:
  free((void *)local_settings->supported_paths);
  free(local_settings);
error_out:
  return err;
}

void cmc_settings_destroy(struct cmc_ConfigSettings **settings) {
  if (!settings || !(*settings)) {
    return;
  }

  free((*settings)->name);

  while ((*settings)->paths_length-- > 0) {
    free((*settings)->supported_paths[(*settings)->paths_length]);
  }

  free((void *)(*settings)->supported_paths);

  free(*settings);

  *settings = NULL;
}
