/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_minilib_config.h"
#include "cmc_env_parser.h"
#include "cmc_parse_interface/cmc_parse_interface.h"
#include "utils/cmc_common.h"
#include "utils/cmc_field.h"
#include "utils/cmc_file.h"
#include "utils/cmc_string.h"
#include "utils/cmc_tree.h"

static const char *cmc_env_parser_extension = ".env";
static cme_error_t cmc_env_parser_create(cmc_ConfigParserData *data);
static cme_error_t cmc_env_parser_is_format(const size_t n, const char path[n],
                                            bool *result);
static cme_error_t cmc_env_parser_parse(const size_t n, const char path[n],
                                        const cmc_ConfigParserData data,
                                        struct cmc_Config *config);
static void cmc_env_parser_destroy(cmc_ConfigParserData *data);
static cme_error_t
cmc_env_parser_parse_field(FILE *config_file, struct cmc_ConfigField *field,
                           bool *found_value,
                           struct cmc_ConfigSettings *settings);
static cme_error_t cmc_env_parser_parse_str_and_int_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value,
    struct cmc_ConfigSettings *settings);
static cme_error_t
cmc_env_parser_parse_single_line(char *buffer,        // NOLINT
                                 uint32_t buffer_max, // NOLINT
                                 char **name,         // NOLINT
                                 char **value);       // NOLINT
static cme_error_t cmc_env_parser_parse_array_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value,
    struct cmc_ConfigSettings *settings);
static char *cmc_env_parser_create_array_name(const char *base_name,
                                              int32_t index);
static cme_error_t cmc_field_deep_clone(struct cmc_ConfigField *src,
                                        struct cmc_ConfigField **dst);
static void cmc_field_last_destroy(struct cmc_ConfigField *field);
static cme_error_t cmc_env_parser_parse_dict_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value,
    struct cmc_ConfigSettings *settings);
static char *cmc_env_parser_create_dict_name(const char *dict_name,
                                             const char *key);

cme_error_t cmc_env_parser_init(struct cmc_ConfigParseInterface *parser) {
  struct cmc_ConfigParseInterface env_parser = {
      .id = "env",
      .create = cmc_env_parser_create,
      .is_format = cmc_env_parser_is_format,
      .parse = cmc_env_parser_parse,
      .destroy = cmc_env_parser_destroy,
  };

  *parser = env_parser;

  return NULL;
};

static cme_error_t cmc_env_parser_create(cmc_ConfigParserData *data) {
  return NULL;
};

static cme_error_t cmc_env_parser_is_format(const size_t n, const char path[n],
                                            bool *result) {
  char new_file_path[PATH_MAX];

  cmc_join_str_stack(new_file_path, sizeof(new_file_path), path,
                     cmc_env_parser_extension);

  *result = cmc_does_file_exist(new_file_path);

  return NULL;
}

static cme_error_t cmc_env_parser_parse(const size_t n, const char path[n],
                                        const cmc_ConfigParserData data,
                                        struct cmc_Config *config) {

  cme_error_t err;
  char file_path[PATH_MAX];

  cmc_join_str_stack(file_path, sizeof(file_path), path,
                     cmc_env_parser_extension);

  FILE *config_file = fopen(file_path, "r");
  if (!config_file) {
    err = cme_errorf(EINVAL, "Unable to open %s\n", file_path);
    goto error_out;
  }
  // We need for each field to iterate over a config file.
  //  If field is int or str we try to match it's name.
  //  If field is array we try to match it's `name_N` once,
  //    N is matched it proceeds to `name_N+1` etc. Once
  //    match occurs appropriate value is populated. If we
  //    have nested array than try to match for `name_0_0`
  //    and after that `name_N_P` etc. Once there is no
  //     match we stop looking further.
  CMC_TREE_SUBNODES_FOREACH(node, config->_fields) {
    struct cmc_ConfigField *field = cmc_field_of_node(node);
    bool found_value = false;
    err = cmc_env_parser_parse_field(config_file, field, &found_value,
                                     config->settings);
    if (err) {
      CMC_LOG(config->settings, cmc_LogLevelEnum_DEBUG,               // NOLINT
              "Unable to parse config `file_path=%s`: %s", file_path, // NOLINT
              err->msg);                                              // NOLINT
      goto error_file_cleanup;
    }
  }

  fclose(config_file);

  return NULL;

error_file_cleanup:
  fclose(config_file);
error_out:
  return cme_return(err);
}

static void cmc_env_parser_destroy(cmc_ConfigParserData *data){

};

static cme_error_t
cmc_env_parser_parse_field(FILE *config_file, struct cmc_ConfigField *field,
                           bool *found_value,
                           struct cmc_ConfigSettings *settings) {
  cme_error_t err;

  CMC_LOG(settings, cmc_LogLevelEnum_DEBUG,                      // NOLINT
          "Parsing name=%s, type=%d, children=%d, found=%d",     // NOLINT
          field->name,                                           // NOLINT
          field->type, field->_self.subnodes_len, *found_value); // NOLINT

  switch (field->type) {
  case cmc_ConfigFieldTypeEnum_INT:
  case cmc_ConfigFieldTypeEnum_STRING:
    err = cmc_env_parser_parse_str_and_int_field(config_file, field,
                                                 found_value, settings);
    break;
  case cmc_ConfigFieldTypeEnum_ARRAY:
    err = cmc_env_parser_parse_array_field(config_file, field, found_value,
                                           settings);
    break;
  case cmc_ConfigFieldTypeEnum_DICT:
    err = cmc_env_parser_parse_dict_field(config_file, field, found_value,
                                          settings);
    break;

  default:
    err =
        cme_errorf(EINVAL, "Unrecognized type `field->type=%d`\n", field->type);
  }
  if (err) {
    goto error_out;
  }

  CMC_LOG(settings, cmc_LogLevelEnum_DEBUG,                      // NOLINT
          "Parsed name=%s, type=%d, children=%d, found=%d",      // NOLINT
          field->name,                                           // NOLINT
          field->type, field->_self.subnodes_len, *found_value); // NOLINT

  if (!*found_value && !field->optional) {
    err = cme_errorf(ENODATA, "Required field is missing `field->name=%s`\n",
                     field->name);
    goto error_out;
  }

  return NULL;

error_out:
  return cme_return(err);
}

static cme_error_t cmc_env_parser_parse_str_and_int_field( // NOLINT
    FILE *config_file, struct cmc_ConfigField *field,      // NOLINT
    bool *found_value,                                     // NOLINT
    struct cmc_ConfigSettings *settings) {                 // NOLINT
  const uint32_t single_line_max = 255;
  char single_line_buffer[single_line_max];
  cme_error_t err;

  fseek(config_file, 0, SEEK_SET);

  *found_value = false;
  while (
      fgets(single_line_buffer, (int)single_line_max, config_file) != // NOLINT
      NULL) {                                                         // NOLINT
    if (strlen(single_line_buffer) <= 1) {
      continue;
    }

    char *env_field_name = NULL;
    char *env_field_value = NULL;
    err = cmc_env_parser_parse_single_line(single_line_buffer, single_line_max,
                                           &env_field_name, &env_field_value);
    if (err) {
      free(env_field_name);
      free(env_field_value);
      if (err->code == ENODATA) {
        cme_error_destroy(err);
        continue;
      }
      goto error_out;
    }

    if (!env_field_name || !env_field_value) {
      continue;
    }

    if (strcmp(field->name, env_field_name) == 0) {
      if (field->value) {
        free(field->value);
      }

      int value = -1;
      field->value = NULL;
      switch (field->type) {
      case cmc_ConfigFieldTypeEnum_STRING:
        err = cmc_field_add_value_str(field, env_field_value);
        break;
      case cmc_ConfigFieldTypeEnum_INT:
        err = cmc_convert_str_to_int(env_field_value, strlen(env_field_value),
                                     &value);
        if (err) {
          free(env_field_name);
          free(env_field_value);
          goto error_out;
        }

        err = cmc_field_add_value_int(field, value);
        break;
      default:;
        err = cme_errorf(ENOMEM, "Unrecognized value for `field->type=%d`\n",
                         field->type);
      }

      free(env_field_name);
      free(env_field_value);

      if (err) {
        goto error_out;
      }

      *found_value = true;
      break;
    }

    free(env_field_name);
    free(env_field_value);
  }

  return NULL;

error_out:
  return cme_return(err);
}

static cme_error_t
cmc_env_parser_parse_single_line(char *buffer,                // NOLINT
                                 uint32_t buffer_max,         // NOLINT
                                 char **name, char **value) { // NOLINT
  const char delimeter = '=';
  char *delimeter_ptr;
  cme_error_t err;

  delimeter_ptr = strchr(buffer, delimeter);
  if (!delimeter_ptr) {
    err = cme_errorf(EINVAL, "No `delimeter=%c` found in `line=%s`\n",
                     delimeter, buffer);
    return cme_return(err);
  }

  uint32_t env_field_name_len = delimeter_ptr - buffer;
  char env_field_name[env_field_name_len + 1];
  memset(env_field_name, 0, env_field_name_len + 1);
  strncpy(env_field_name, buffer, env_field_name_len);

  char *name_char;
  CMC_FOREACH_PTR(name_char, env_field_name, strlen(env_field_name)) {
    *name_char = (char)tolower((int)*name_char);
  }

  uint32_t env_field_value_len = strlen(delimeter_ptr + 1);
  char env_field_value[env_field_value_len + 1];
  strncpy(env_field_value, delimeter_ptr + 1, env_field_value_len);
  env_field_value[env_field_value_len] = 0;

  if (strlen(env_field_value) <= 1 || strlen(env_field_name) <= 1) {
    err = cme_errorf(ENODATA, "Unable to find value for `env_field_name=%s`",
                     env_field_name);
    goto error_out;
  }

  if (env_field_value[env_field_value_len - 1] == '\n') {
    env_field_value[env_field_value_len - 1] = 0;
  }

  *name = strdup(env_field_name);
  if (!*name) {
    err =
        cme_errorf(EINVAL, "Unable to allocate memory for `env_field_name=%s`",
                   env_field_name);
    goto error_out;
  }

  *value = strdup(env_field_value);
  if (!*value) {
    err =
        cme_errorf(EINVAL, "Unable to allocate memory for `env_field_value=%s`",
                   env_field_value);
    goto error_out;
  }

  return NULL;

error_out:
  return cme_return(err);
}

static cme_error_t cmc_env_parser_parse_array_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value,
    struct cmc_ConfigSettings *settings) {
  cme_error_t err;

  if (!field) {
    *found_value = false;
    return NULL;
  }

  int32_t index = 0;

  while (true) {
    struct cmc_ConfigField *subfield =
        cmc_field_of_node(field->_self.subnodes[index]);
    /* char **subfield_old_name = subfield->name; */
    char *subfield_new_name =
        cmc_env_parser_create_array_name(field->name, index);
    if (!subfield_new_name) {
      err =
          cme_errorf(EINVAL, "Unable to allocate memory for `array_elem_name`");
      goto error_out;
    }
    free(subfield->name);
    subfield->name = subfield_new_name;

    bool local_found_value = true;
    err = cmc_env_parser_parse_field(config_file, subfield, &local_found_value,
                                     settings);
    if (err) {
      goto error_out;
    }

    if (!local_found_value) {
      break;
    }

    struct cmc_ConfigField *new_node_field = NULL;
    err = cmc_field_deep_clone(subfield, &new_node_field);
    if (err) {
      goto error_out;
    }

    err = cmc_field_add_subfield(field, new_node_field);
    if (err) {
      goto error_out;
    }

    index++;
  }

  if (index > 0) {
    cmc_field_last_destroy(field);
    *found_value = true;

  } else {
    *found_value = false;
  }

  return NULL;

error_out:
  return cme_return(err);
}

static char *cmc_env_parser_create_array_name(const char *base_name,
                                              int32_t index) {
  const size_t buffer_max = 255;
  char buffer[buffer_max];

  int written = snprintf(buffer, buffer_max, "%s_%d", base_name, index);
  if (written < 0 || (size_t)written >= buffer_max) {
    // Truncation or error occurred
    return NULL;
  }

  return strdup(buffer); // Caller must free
}

static cme_error_t cmc_field_deep_clone(struct cmc_ConfigField *src,
                                        struct cmc_ConfigField **dst) {
  cme_error_t err;

  struct cmc_ConfigField *copy = NULL;

  err =
      cmc_field_create(src->name, src->type, src->value, src->optional, &copy);
  if (err) {
    goto error_out;
  }

  CMC_FOREACH_FIELD(subfield, src, {
    struct cmc_ConfigField *subfield_cp;
    err = cmc_field_deep_clone(subfield, &subfield_cp);
    if (err) {
      goto error_out;
    }

    cmc_field_add_subfield(copy, subfield_cp);

    // For array we need to copy only first element, other elements
    //  will be filled by parse array func.
    if (src->type == cmc_ConfigFieldTypeEnum_ARRAY) {
      break;
    }
  })

  *dst = copy;

  return NULL;
error_out:
  return cme_return(err);
}

static void cmc_field_last_destroy(struct cmc_ConfigField *field) {
  cme_error_t err;

  struct cmc_ConfigField *last_subfield =
      cmc_field_of_node(field->_self.subnodes[field->_self.subnodes_len - 1]);
  cmc_field_destroy(&last_subfield);

  err = cmc_tree_node_pop_subnode(&field->_self);
  if (err) {
    cme_error_destroy(err);
    return;
  }
}

static cme_error_t cmc_env_parser_parse_dict_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value,
    struct cmc_ConfigSettings *settings) {
  int32_t found_i = 0;
  cme_error_t err;

  CMC_FOREACH_FIELD(subfield, field, {
    char *old_subfield_name = subfield->name;
    char *new_subfield_name =
        cmc_env_parser_create_dict_name(field->name, subfield->name);
    subfield->name = new_subfield_name;

    bool local_found_value = true;
    err = cmc_env_parser_parse_field(config_file, subfield, &local_found_value,
                                     settings);
    subfield->name = old_subfield_name;
    free(new_subfield_name);
    if (err) {
      goto error_out;
    }

    if (local_found_value) {
      found_i++;
    }
  })

  if (found_i > 0) {
    *found_value = true;
  } else {
    *found_value = false;
  }

  return NULL;

error_out:
  return cme_return(err);
}

static char *cmc_env_parser_create_dict_name(const char *dict_name,
                                             const char *key) {
  const size_t buffer_max = 255;
  char buffer[buffer_max];

  int written = snprintf(buffer, buffer_max, "%s_%s", dict_name, key);
  if (written < 0 || (size_t)written >= buffer_max) {
    return NULL;
  }

  return strdup(buffer);
}
