#include <asm-generic/errno.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_minilib_config.h"
#include "cmc_env_parser.h"
#include "cmc_parse_interface/cmc_parse_interface.h"
#include "utils/cmc_common.h"
#include "utils/cmc_error.h"
#include "utils/cmc_field.h"
#include "utils/cmc_file.h"
#include "utils/cmc_string.h"
#include "utils/cmc_tree.h"

static const char *cmc_env_parser_extension = ".env";
static cmc_error_t _cmc_env_parser_create(cmc_ConfigParserData *data);
static cmc_error_t _cmc_env_parser_is_format(const size_t n, const char path[n],
                                             bool *result);
static cmc_error_t _cmc_env_parser_parse(const size_t n, const char path[n],
                                         const cmc_ConfigParserData data,
                                         struct cmc_Config *config);
static void _cmc_env_parser_destroy(cmc_ConfigParserData *data);
static cmc_error_t _cmc_env_parser_parse_field(FILE *config_file,
                                               struct cmc_ConfigField *field,
                                               bool *found_value);
static cmc_error_t _cmc_env_parser_parse_str_and_int_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value);
static cmc_error_t _cmc_env_parser_parse_field(FILE *config_file,
                                               struct cmc_ConfigField *field,
                                               bool *found_value);
static cmc_error_t _cmc_env_parser_parse_single_line(char *buffer,
                                                     uint32_t buffer_max,
                                                     char **env_name,
                                                     char **env_value);
static cmc_error_t _cmc_env_parser_parse_array_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value);
static char *_cmc_env_parser_create_array_name(char *name, int32_t i);
static cmc_error_t _cmc_field_deep_clone(struct cmc_ConfigField *src,
                                         struct cmc_ConfigField **dst);
static void _cmc_field_last_destroy(struct cmc_ConfigField *field);
static cmc_error_t _cmc_env_parser_parse_dict_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value);
static char *_cmc_env_parser_create_dict_name(char *dict_name, char *key);

cmc_error_t cmc_env_parser_init(struct cmc_ConfigParseInterface *parser) {
  struct cmc_ConfigParseInterface env_parser = {
      .id = "env",
      .create = _cmc_env_parser_create,
      .is_format = _cmc_env_parser_is_format,
      .parse = _cmc_env_parser_parse,
      .destroy = _cmc_env_parser_destroy,
  };

  memcpy(parser, &env_parser, sizeof(struct cmc_ConfigParseInterface));

  return NULL;
};

static cmc_error_t _cmc_env_parser_create(cmc_ConfigParserData *data) {
  return NULL;
};

static cmc_error_t _cmc_env_parser_is_format(const size_t n, const char path[n],
                                             bool *result) {

  CMC_JOIN_STR_STACK(new_file_path, path, cmc_env_parser_extension);

  *result = cmc_does_file_exist(new_file_path);

  return NULL;
}

static cmc_error_t _cmc_env_parser_parse(const size_t n, const char path[n],
                                         const cmc_ConfigParserData data,
                                         struct cmc_Config *config) {

  cmc_error_t err;

  CMC_JOIN_STR_STACK(file_path, path, cmc_env_parser_extension);

  FILE *config_file = fopen(file_path, "r");
  if (!config_file) {
    err = cmc_errorf(EINVAL, "Unable to open %s\n", file_path);
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
    err = _cmc_env_parser_parse_field(config_file, field, &found_value);
    if (err) {
      CMC_LOG(config->settings, cmc_LogLevelEnum_DEBUG,
              "Unable to parse config `file_path=%s`: %s", file_path, err->msg);
      goto error_file_cleanup;
    }
  }

  fclose(config_file);

  return NULL;

error_file_cleanup:
  fclose(config_file);
error_out:
  return err;
}

static void _cmc_env_parser_destroy(cmc_ConfigParserData *data){

};

static cmc_error_t _cmc_env_parser_parse_field(FILE *config_file,
                                               struct cmc_ConfigField *field,
                                               bool *found_value) {
  cmc_error_t err;

  printf("Parsing name=%s, type=%d, children=%d, found=%d\n", field->name,
         field->type, field->_self.subnodes_len, *found_value);

  switch (field->type) {
  case cmc_ConfigFieldTypeEnum_INT:
  case cmc_ConfigFieldTypeEnum_STRING:
    err = _cmc_env_parser_parse_str_and_int_field(config_file, field,
                                                  found_value);
    break;
  case cmc_ConfigFieldTypeEnum_ARRAY:
    err = _cmc_env_parser_parse_array_field(config_file, field, found_value);
    break;
  case cmc_ConfigFieldTypeEnum_DICT:
    err = _cmc_env_parser_parse_dict_field(config_file, field, found_value);
    break;

  default:
    err =
        cmc_errorf(EINVAL, "Unrecognized type `field->type=%d`\n", field->type);
  }
  if (err) {
    goto error_out;
  }

  printf("Parsed name=%s, type=%d, children=%d, found=%d\n", field->name,
         field->type, field->_self.subnodes_len, *found_value);

  if (!*found_value && !field->optional) {
    err = cmc_errorf(ENODATA, "Required field is missing `field->name=%s`\n",
                     field->name);
    goto error_out;
  }

  return NULL;

error_out:
  return err;
}

static cmc_error_t _cmc_env_parser_parse_str_and_int_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value) {
  const uint32_t single_line_max = 255;
  char single_line_buffer[single_line_max];
  cmc_error_t err;

  fseek(config_file, 0, SEEK_SET);

  *found_value = false;
  while (fgets(single_line_buffer, (int)single_line_max, config_file) != NULL) {
    if (strlen(single_line_buffer) <= 1) {
      continue;
    }

    char *env_field_name;
    char *env_field_value;
    err = _cmc_env_parser_parse_single_line(single_line_buffer, single_line_max,
                                            &env_field_name, &env_field_value);
    if (err) {
      if (err->code == ENODATA) {
        cmc_error_destroy(&err);
        continue;
      }
      goto error_out;
    }

    printf("env_field_name=%s, field_name=%s\n", env_field_name, field->name);
    if (strcmp(field->name, env_field_name) == 0) {
      if (field->value) {
        free(field->value);
      }

      field->value = NULL;
      switch (field->type) {
      case cmc_ConfigFieldTypeEnum_STRING:
        err = cmc_field_add_value_str(field, env_field_value);
        break;
      case cmc_ConfigFieldTypeEnum_INT:
        err = cmc_field_add_value_int(field, atoi(env_field_value));
        break;
      default:;
        err = cmc_errorf(ENOMEM, "Unrecognized value for `field->type=%d`\n",
                         field->type);
      }

      free(env_field_name);
      env_field_name = NULL;
      free(env_field_value);
      env_field_value = NULL;

      if (err) {
        goto error_out;
      }

      *found_value = true;
      break;
    }

    free(env_field_name);
    env_field_name = NULL;
    free(env_field_value);
    env_field_value = NULL;
  }

  return NULL;

error_out:
  return err;
}

static cmc_error_t _cmc_env_parser_parse_single_line(char *buffer,
                                                     uint32_t buffer_max,
                                                     char **env_name,
                                                     char **env_value) {
  const char delimeter = '=';
  char *delimeter_ptr;
  cmc_error_t err;

  delimeter_ptr = strchr(buffer, delimeter);
  if (!delimeter_ptr) {
    err = cmc_errorf(EINVAL, "No `delimeter=%c` found in `line=%s`\n",
                     delimeter, buffer);
    return err;
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
    err = cmc_errorf(ENODATA, "Unable to find value for `env_field_name=%s`",
                     env_field_name);
    return err;
  }

  if (env_field_value[env_field_value_len - 1] == '\n') {
    env_field_value[env_field_value_len - 1] = 0;
  }

  *env_name = strdup(env_field_name);
  if (!*env_name) {
    err =
        cmc_errorf(EINVAL, "Unable to allocate memory for `env_field_name=%s`",
                   env_field_name);
    return err;
  }

  *env_value = strdup(env_field_value);
  if (!*env_value) {
    err =
        cmc_errorf(EINVAL, "Unable to allocate memory for `env_field_value=%s`",
                   env_field_value);
    goto error_out;
  }

  return NULL;

error_out:
  return err;
}

static cmc_error_t _cmc_env_parser_parse_array_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value) {
  cmc_error_t err;

  if (!field) {
    *found_value = false;
    return NULL;
  }

  int32_t i = 0;

  while (true) {
    struct cmc_ConfigField *subfield =
        cmc_field_of_node(field->_self.subnodes[i]);
    /* char **subfield_old_name = subfield->name; */
    char *subfield_new_name = _cmc_env_parser_create_array_name(field->name, i);
    if (!subfield_new_name) {
      err =
          cmc_errorf(EINVAL, "Unable to allocate memory for `array_elem_name`");
      goto error_out;
    }
    free(subfield->name);
    subfield->name = subfield_new_name;

    bool local_found_value = true;
    err =
        _cmc_env_parser_parse_field(config_file, subfield, &local_found_value);
    if (err) {
      goto error_out;
    }

    if (!local_found_value) {
      break;
    }

    struct cmc_ConfigField *new_node_field = NULL;
    err = _cmc_field_deep_clone(subfield, &new_node_field);
    if (err) {
      goto error_out;
    }

    err = cmc_field_add_subfield(field, new_node_field);
    if (err) {
      goto error_out;
    }

    i++;
  }

  if (i > 0) {
    _cmc_field_last_destroy(field);
    *found_value = true;

  } else {
    *found_value = false;
  }

  return NULL;

error_out:
  return err;
}

static char *_cmc_env_parser_create_array_name(char *name, int32_t i) {
  const uint32_t buffer_max = 255;
  char buffer[buffer_max];

  memset(buffer, 0, buffer_max);
  snprintf(buffer, buffer_max, "%s_%d", name, i);

  return strdup(buffer);
};

static cmc_error_t _cmc_field_deep_clone(struct cmc_ConfigField *src,
                                         struct cmc_ConfigField **dst) {
  cmc_error_t err;

  struct cmc_ConfigField *copy = NULL;

  err =
      cmc_field_create(src->name, src->type, src->value, src->optional, &copy);
  if (err) {
    return err;
  }

  CMC_FIELD_FOREACH(subfield, src, {
    struct cmc_ConfigField *subfield_cp;
    err = _cmc_field_deep_clone(subfield, &subfield_cp);
    if (err) {
      return err;
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
}

static void _cmc_field_last_destroy(struct cmc_ConfigField *field) {
  cmc_error_t err;

  struct cmc_ConfigField *last_subfield =
      cmc_field_of_node(field->_self.subnodes[field->_self.subnodes_len - 1]);
  cmc_field_destroy(&last_subfield);

  err = cmc_tree_node_pop_subnode(&field->_self);
  if (err) {
    return;
  }
}

static cmc_error_t _cmc_env_parser_parse_dict_field(
    FILE *config_file, struct cmc_ConfigField *field, bool *found_value) {
  int32_t found_i = 0;
  cmc_error_t err;

  CMC_FIELD_FOREACH(subfield, field, {
    char *old_subfield_name = subfield->name;
    char *new_subfield_name =
        _cmc_env_parser_create_dict_name(field->name, subfield->name);
    subfield->name = new_subfield_name;

    bool local_found_value = true;
    err =
        _cmc_env_parser_parse_field(config_file, subfield, &local_found_value);
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
  return err;
}

static char *_cmc_env_parser_create_dict_name(char *dict_name, char *key) {
  const uint32_t buffer_max = 255;
  char buffer[buffer_max];

  memset(buffer, 0, buffer_max);
  snprintf(buffer, buffer_max, "%s_%s", dict_name, key);

  return strdup(buffer);
};
