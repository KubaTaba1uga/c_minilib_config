/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <limits.h>
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
#include "utils/cmc_field.h"
#include "utils/cmc_settings.h"
#include "utils/cmc_string.h"
#include "utils/cmc_tree.h"

static struct cmc_ConfigParseInterface parsers[cmc_ConfigParseFormat_MAX];
static int32_t parsers_length = 0;

cme_error_t cmc_lib_init(void) {

  cme_error_t err;

  if (parsers_length != 0) {
    return NULL;
  }

  if (cme_init() != 0) {
    err = cme_error(ENOMEM, "Unable to initialize c minilib error");
    goto error_out;
  }

  err = cmc_env_parser_init(&parsers[parsers_length++]);
  if (err) {
    goto error_out;
  }

  return NULL;

error_out:
  return cme_return(err);
};

void cmc_lib_destroy(void) { cme_destroy(); }

cme_error_t cmc_config_create(const struct cmc_ConfigSettings *settings,
                              struct cmc_Config **config) {
  struct cmc_Config *local_config;
  cme_error_t err;

  if (!config) {
    err = cme_error(EINVAL, "`config` cannot be NULL");
    goto error_out;
  }

  local_config = malloc(sizeof(struct cmc_Config));
  if (!local_config) {
    err = cme_error(ENOMEM, "Unable to allocate memory for `local_config`");
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
  return cme_return(err);
};

void cmc_config_destroy(struct cmc_Config **config) {
  if (!config || !*config) {
    return;
  }

  cmc_settings_destroy(&(*config)->settings);

  CMC_TREE_SUBNODES_FOREACH(subnode, (*config)->_fields) {
    struct cmc_ConfigField *subfield = cmc_field_of_node(subnode);
    cmc_field_destroy(&subfield);
  }

  cmc_tree_node_destroy(&(*config)->_fields);

  free(*config);

  *config = NULL;
};

cme_error_t cmc_config_add_field(struct cmc_ConfigField *field,
                                 struct cmc_Config *config) {
  cme_error_t err;

  if (!field || !config) {
    err = cme_error(EINVAL, "`field` and `config` cannot be NULL");
    goto error_out;
  }

  err = cmc_tree_node_add_subnode(&field->_self, &config->_fields);
  if (err) {
    goto error_out;
  }

  return NULL;

error_out:
  return cme_return(err);
};

cme_error_t cmc_config_parse(struct cmc_Config *config) { // NOLINT
  struct cmc_ConfigParseInterface *parser;
  cme_error_t err;

  if (!config) {
    err = cme_error(EINVAL, "`config` cannot be NULL");
    goto error_out;
  }

  CMC_LOG(config->settings, cmc_LogLevelEnum_DEBUG, // NOLINT
          "Starting configuration file parsing");   // NOLINT

  bool matched_parser = false;

  CMC_FOREACH_PTR(parser, parsers, parsers_length) {
    err = parser->create((cmc_ConfigParserData *)parser->data);
    if (err) {
      goto error_out;
    }

    /* Search supported paths to find matching configuration file. */
    /* Once we have first match search is stopped and parsing occurs. */
    char *dir_path;
    char file_path[PATH_MAX];
    CMC_FOREACH(dir_path, config->settings->supported_paths,
                config->settings->paths_length) {
      cmc_join_path_stack(file_path, sizeof(file_path), dir_path,
                          config->settings->name);

      CMC_LOG(config->settings, cmc_LogLevelEnum_DEBUG,     // NOLINT
              "Checking %s configuration file", file_path); // NOLINT

      err = parser->is_format(sizeof(file_path) / sizeof(char), file_path,
                              &matched_parser);
      if (err) {
        return cme_return(err);
      }

      if (matched_parser) {
        CMC_LOG(config->settings, cmc_LogLevelEnum_DEBUG,  // NOLINT
                "Using %s configuration file", file_path); // NOLINT

        err = parser->parse(sizeof(file_path) / sizeof(char), file_path,
                            parser->data, config);
        if (err) {
          parser->destroy((cmc_ConfigParserData *)parser->data);
          goto error_out;
        }

        // TO-DO: print all configuration fields
        break;
      }
    }

    parser->destroy((cmc_ConfigParserData *)parser->data);

    if (matched_parser) {
      CMC_LOG(config->settings, cmc_LogLevelEnum_DEBUG, // NOLINT
              "Finished configuration file parsing");   // NOLINT
      break;
    }
  }

  if (!matched_parser) {
    err = cme_error(ENOENT, "Unable to find configuration file");
    goto error_out;
  }

  return NULL;

error_out:
  return cme_return(err);
};
