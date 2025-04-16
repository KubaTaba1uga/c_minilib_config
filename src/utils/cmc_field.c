#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "c_minilib_config.h"
#include "utils/cmc_field.h"

static void add_default_value(struct cmc_ConfigField *field,
                              const void *default_value) {
  int32_t *default_int;
  switch (field->type) {
  case cmc_ConfigFieldTypeEnum_STRING:
    field->default_value = strdup(default_value);
    break;
  case cmc_ConfigFieldTypeEnum_INT:
    default_int = malloc(sizeof(int32_t));
    *default_int = *(int32_t *)default_value;
    field->default_value = default_int;
    break;
  default:;
  }
}

cmc_error_t cmc_field_create(const char *name,
                             const enum cmc_ConfigFieldTypeEnum type,
                             const void *default_value, const bool optional,
                             struct cmc_ConfigField **field) {
  struct cmc_ConfigField *local_field;
  cmc_error_t err;

  if (!name || !field) {
    err = cmc_errorf(EINVAL, "`name=%p` and `field=%p` cannot be NULL\n", name,
                     field);
    goto error_out;
  }

  if (type <= cmc_ConfigFieldTypeEnum_NONE ||
      type >= cmc_ConfigFieldTypeEnum_MAX) {
    err = cmc_errorf(EINVAL, "`type=%d` unrecognized\n", type);
    goto error_out;
  }

  if (optional && !default_value) {
    err = cmc_errorf(EINVAL, "`default_value=%p` cannot be NULL\n",
                     default_value);
    goto error_out;
  }

  local_field = malloc(sizeof(struct cmc_ConfigField));
  if (!local_field) {
    err = cmc_errorf(ENOMEM,
                     "Unable to allocate memory for `local_field`, "
                     "requested memory size: %d\n",
                     sizeof(struct cmc_ConfigField));
    goto error_out;
  }

  local_field->name = strdup(name);
  if (!local_field->name) {
    err = cmc_errorf(ENOMEM,
                     "Unable to allocate memory for `local_field->name`\n");
    goto error_field_cleanup;
  }

  local_field->optional = optional;
  local_field->next_field = NULL;
  local_field->value = NULL;
  local_field->type = type;

  add_default_value(local_field, default_value);

  *field = local_field;

  return NULL;

error_field_cleanup:
  free(local_field);
error_out:
  return err;
};

void cmc_field_destroy(struct cmc_ConfigField **field) {
  if (!field || !*field) {
    return;
  }

  free((*field)->default_value);
  free((*field)->name);
  free(*field);
  *field = NULL;
};
