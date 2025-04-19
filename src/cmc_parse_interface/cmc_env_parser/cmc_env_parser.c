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
static cmc_error_t _cmc_env_parser_parse_field(FILE *config_file,
                                               struct cmc_ConfigField *field);

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
  CMC_FIELD_ITER(field, config->fields) {
    err = _cmc_env_parser_parse_field(config_file, field);
    if (err) {
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
                                               struct cmc_ConfigField *field) {
  const char delimeter = '=';
  char buffer[255];
  cmc_error_t err;

  if (field->type == cmc_ConfigFieldTypeEnum_ARRAY) {
    struct cmc_ConfigField *subfield = field->value;

    for (int32_t i = 0;; i++) {
      snprintf(buffer, 255, "%s_%d", field->name, i);
      subfield->name = buffer;

      err = _cmc_env_parser_parse_field(config_file, subfield);
      if (err) {
        if (err->code == ENOENT) {
          cmc_error_destroy(&err);
          goto out;
        }

        goto error_out;
      }

      subfield->next_field = calloc(sizeof(struct cmc_ConfigField), 1);
      subfield = subfield->next_field;
    }
  }

  fseek(config_file, 0, SEEK_SET);

  if (field->type == cmc_ConfigFieldTypeEnum_STRING ||
      field->type == cmc_ConfigFieldTypeEnum_INT) {
    while (fgets(buffer, 255, config_file) != NULL) {
      char *delimeter_ptr = strchr(buffer, delimeter);
      if (!delimeter_ptr) {
        err = cmc_errorf(EINVAL, "No `delimeter=%c` found in `line=%s`\n",
                         delimeter, buffer);
        goto error_out;
      }

      char env_field_name[delimeter_ptr - buffer];
      memset(env_field_name, 0, delimeter_ptr - buffer);
      strncpy(env_field_name, delimeter_ptr + 1, delimeter_ptr - buffer);
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
          continue;
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
      }
    }

    if (!field->optional) {
      err = cmc_errorf(ENOENT, "No env with name `field->name=%s`\n",
                       field->name);
      goto error_out;
    }
  }

out:
  return NULL;

error_out:
  return err;
}

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
