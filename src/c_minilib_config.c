#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <c_minilib_config.h>
#include <c_minilib_error.h>

#include "cmc_parse_interface/cmc_env_parser/cmc_env_parser.h"
#include "cmc_parse_interface/cmc_parse_interface.h"
#include "utils/cmc_common.h"
#include "utils/cmc_error.h"
#include "utils/cmc_field.h"
#include "utils/cmc_settings.h"
#include "utils/cmc_string.h"
#include "utils/cmc_tree.h"

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
    err = cmc_errorf(EINVAL, "`config=%p` cannot be NULL\n", config);
    goto error_out;
  }

  local_config = malloc(sizeof(struct cmc_Config));
  if (!local_config) {
    err = cmc_errorf(ENOMEM, "Unable to allocate memory for `local_config`\n");
    goto error_out;
  }

  err = cmc_tree_node_create(&local_config->_fields);
  if (err) {
    goto error_config_cleanup;
  }

  if (!settings) {
    err = cmc_settings_create(0, NULL, NULL, NULL, &local_config->settings);
  } else {
    err = cmc_settings_create(
        settings->paths_length, (const char **)settings->supported_paths,
        settings->name, settings->log_func, &local_config->settings);
  }
  if (err) {
    goto error_config_cleanup;
  }

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

  CMC_TREE_SUBNODES_ITER(subnode, (*config)->_fields) {
    struct cmc_ConfigField *subfield = cmc_field_of_node(subnode);
    cmc_field_destroy(&subfield);
  }

  cmc_tree_node_destroy(&(*config)->_fields);

  free(*config);

  *config = NULL;
};

cmc_error_t cmc_config_add_field(struct cmc_ConfigField *field,
                                 struct cmc_Config *config) {
  cmc_error_t err;

  if (!field || !config) {
    err = cmc_errorf(EINVAL, "`field=%p` and `config=%p` cannot be NULL\n",
                     field, config);
    goto error_out;
  }

  err = cmc_tree_node_add_subnode(&field->_self, &config->_fields);
  if (err) {
    goto error_out;
  }

  return NULL;

error_out:
  return err;
};

cmc_error_t cmc_config_parse(struct cmc_Config *config) {
  struct cmc_ConfigParseInterface *parser;
  cmc_error_t err;

  if (!config) {
    err = cmc_errorf(EINVAL, "`config=%p` cannot be NULL\n", config);
    goto error_out;
  }

  CMC_LOG(config->settings, cmc_LogLevelEnum_DEBUG,
          "Starting configuration file parsing");

  bool matched_parser = false;

  CMC_FOREACH_PTR(parser, parsers, parsers_length) {
    err = parser->create(parser->data);
    if (err) {
      goto error_out;
    }

    /* Search supported paths to find matching configuration file. */
    /* Once we have first match search is stopped and parsing occurs. */
    char *dir_path;
    CMC_FOREACH(dir_path, config->settings->supported_paths,
                config->settings->paths_length) {
      CMC_JOIN_PATH_STACK(file_path, dir_path, config->settings->name);

      CMC_LOG(config->settings, cmc_LogLevelEnum_DEBUG,
              "Checking %s configuration file", file_path);

      err = parser->is_format(sizeof(file_path) / sizeof(char), file_path,
                              &matched_parser);
      if (err) {
        return err;
      }

      if (matched_parser) {
        CMC_LOG(config->settings, cmc_LogLevelEnum_DEBUG,
                "Using %s configuration file", file_path);

        err = parser->parse(sizeof(file_path) / sizeof(char), file_path,
                            parser->data, config);
        if (err) {
          parser->destroy(parser->data);
          goto error_out;
        }

        // TO-DO: print all configuration fields
        break;
      }
    }

    parser->destroy(parser->data);

    if (matched_parser) {
      CMC_LOG(config->settings, cmc_LogLevelEnum_DEBUG,
              "Finished configuration file parsing");
      break;
    }
  }

  if (!matched_parser) {
    err = cmc_errorf(ENOENT, "Unable to find configuration file\n", config);
    goto error_out;
  }

  return NULL;

error_out:
  return err;
};
