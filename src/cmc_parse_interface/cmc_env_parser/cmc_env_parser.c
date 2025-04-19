#include <asm-generic/errno-base.h>
#include <ctype.h>
#include <errno.h>
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

static const char *cmc_env_parser_extension = ".env";
static cmc_error_t _cmc_env_parser_parse_single_line(char *,
                                                     struct cmc_Config *);

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
  //  If field is int or str it tries to match it's name.
  //  If field is array it tries to match it's `name_0` once,
  //    0 is matched it proceeds to `name_1` etc. Once match
  //    occurs appropriate value is populated. If we have
  //    nested array than try to match for `name_0_0` and
  //    after that `name_0_1` etc. Once there is no match
  //    we stop looking further.

  char _buffer[255];
  char *buffer = _buffer;
  while (fgets(buffer, 255, config_file) != NULL) {
    err = _cmc_env_parser_parse_single_line(buffer, config);
    if (err) {
      goto error_out;
    }
  }

  return NULL;

error_out:
  return err;
}

static void _cmc_env_parser_destroy(cmc_ConfigParserData *data){

};

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

static cmc_error_t
_cmc_env_parser_parse_single_line(char *line, struct cmc_Config *config) {
  char delimeter = '=';
  cmc_error_t err;

  char *delimeter_ptr = strchr(line, delimeter);
  if (!delimeter_ptr) {
    err = cmc_errorf(EINVAL, "No `delimeter=%c` found in `line=%s`\n",
                     delimeter, line);
    goto error_out;
  }

  // On left side of delimeter we have field name
  char *env_field_name = strndup(line, delimeter_ptr - line);
  if (!env_field_name) {
    err =
        cmc_errorf(ENOMEM, "unable to allocate memory for `env_field_name`\n");
    goto error_out;
  }

  char *name_char;
  CMC_FOREACH_PTR(name_char, env_field_name, strlen(env_field_name)) {
    *name_char = (char)tolower((int)*name_char);
  }

  // Add str and int
  struct cmc_ConfigField *field = config->fields;
  while (field) {
    if (strcmp(field->name, env_field_name) == 0) {
      // Once we have a match we need to add it's value
      //   with respect to type and stop processing.
      uint32_t value_len = strlen(delimeter_ptr + 1);
      char env_field_value[value_len + 1];
      strncpy(env_field_value, delimeter_ptr + 1, value_len);
      env_field_value[value_len] = 0;

      // If env has empty value we are considering it as non existsent
      //  in config file.
      if (value_len == 0) {
        field->value = NULL;
        goto next_field_cleanup;
      }

      if (env_field_value[value_len - 1] == '\n') {
        env_field_value[value_len - 1] = 0;
      }

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
      if (err) {
        goto error_env_field_name_cleanup;
      }

    next_field_cleanup:
      field = NULL;
    } else {
      field = field->next_field;
    }
  }

  // Add array
  field = config->fields;
  while (field) {
    if (field->type != cmc_ConfigFieldTypeEnum_ARRAY) {
      continue;
    }

    field = field->next_field;
  }

  free(env_field_name);

  return NULL;

error_env_field_name_cleanup:
  free(env_field_name);
error_out:
  return err;
};

cmc_error_t cmc_env_parser_populate_value(struct cmc_Config *config,
                                          char *env_field_name,
                                          char *delimeter_ptr) {
  cmc_error_t err;

  // Add str and int
  struct cmc_ConfigField *field = config->fields;
  while (field) {
    if (strcmp(field->name, env_field_name) == 0) {
      // Once we have a match we need to add it's value
      //   with respect to type and stop processing.
      uint32_t value_len = strlen(delimeter_ptr + 1);
      char env_field_value[value_len + 1];
      strncpy(env_field_value, delimeter_ptr + 1, value_len);
      env_field_value[value_len] = 0;

      // If env has empty value we are considering it as non existsent
      //  in config file.
      if (value_len == 0) {
        field->value = NULL;
        goto next_field_cleanup;
      }

      if (env_field_value[value_len - 1] == '\n') {
        env_field_value[value_len - 1] = 0;
      }

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
      if (err) {
        goto error_out;
      }

    next_field_cleanup:
      field = NULL;
    } else {
      field = field->next_field;
    }
  }

  return NULL;
error_out:
  return err;
}

cmc_error_t
cmc_env_parser_find_matching_field(struct cmc_ConfigField *field,
                                   char *target_field_name,
                                   struct cmc_ConfigField **target_field) {
  cmc_error_t err;

  // field is NULL when cannot find field
  if (!field) {
    *target_field = NULL;
    goto out;
  }

  if (!target_field_name || !target_field) {
    err = cmc_errorf(EINVAL,
                     "`target_field_name=%p` and `target_field=%p` "
                     "cannot be NULL\n",
                     target_field_name, target_field);
    goto error_out;
  }

  if (field->type == cmc_ConfigFieldTypeEnum_INT ||
      field->type == cmc_ConfigFieldTypeEnum_STRING) {
    if (strcmp(field->name, target_field_name) == 0) {
      *target_field = field;
      goto out;
    }

    return cmc_env_parser_find_matching_field(field->next_field,
                                              target_field_name, target_field);
  }

  if (field->type == cmc_ConfigFieldTypeEnum_ARRAY) {
    if (strncmp(field->name, target_field_name, strlen(field->name)) != 0) {
      *target_field = NULL;
      goto out;
    }

    char *arr_i_str = target_field_name + strlen(field->name) + 2;
    if (*(arr_i_str - 1) == '_' && isdigit(*arr_i_str)) {
      char *digit_end = strchr(arr_i_str + 2, '_');
      int32_t arr_i_int;
      if (!digit_end) {
        digit_end = strchr(arr_i_str, 0);
      }
      if (!digit_end) {
        err = cmc_errorf(EINVAL, "Invalid index for array element\n");
        return err;
      }

      err =
          cmc_convert_str_to_int(arr_i_str, digit_end - arr_i_str, &arr_i_int);
      if (err) {
        return err;
      }
    }
  }

  err = cmc_errorf(ENOMEM, "Unrecognized value for `field->type=%d`\n",
                   field->type);
error_out:
  return err;

out:
  return NULL;
}
