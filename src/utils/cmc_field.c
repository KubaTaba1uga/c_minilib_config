#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "c_minilib_config.h"
#include "utils/cmc_field.h"

static inline cmc_error_t cmc_alloc_field_value_str(const char *value,
                                                    void **field_value);
static inline cmc_error_t cmc_alloc_field_value_int(const int32_t value,
                                                    void **field_value);
static inline cmc_error_t
cmc_field_add_default_value(struct cmc_ConfigField *field,
                            const void *default_value);

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

  switch (type) {
  case cmc_ConfigFieldTypeEnum_INT:
  case cmc_ConfigFieldTypeEnum_STRING:
    if (!optional) {
      default_value = NULL;
    }
    break;
  case cmc_ConfigFieldTypeEnum_ARRAY:
    default_value = NULL;
    break;
  default:
    err = cmc_errorf(EINVAL, "`type=%d` unrecognized\n", type);
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

  err = cmc_field_add_default_value(local_field, default_value);
  if (err) {
    goto error_field_name_cleanup;
  }

  *field = local_field;

  return NULL;

error_field_name_cleanup:
  free(local_field->name);
error_field_cleanup:
  free(local_field);
error_out:
  return err;
};

cmc_error_t cmc_field_add_nested_field(struct cmc_ConfigField *field,
                                       struct cmc_ConfigField *child_field) {
  cmc_error_t err;

  if (!field || !child_field) {
    err = cmc_errorf(EINVAL, "`field=%p` and `child_field=%p` cannot be NULL\n",
                     field, child_field);
    goto error_out;
  }

  switch (field->type) {
  case cmc_ConfigFieldTypeEnum_ARRAY:
    break;
  default:
    err = cmc_errorf(EINVAL, "`field->type=%d` cannot be used as container\n",
                     field->type);
    goto error_out;
  }

  field->value = child_field;

  return NULL;
error_out:
  return err;
}

cmc_error_t cmc_field_add_next_field(struct cmc_ConfigField *field,
                                     struct cmc_ConfigField *next_field) {
  cmc_error_t err;

  if (!field || !next_field) {
    err = cmc_errorf(EINVAL, "`field=%p` and `next_field=%p` cannot be NULL\n",
                     field, next_field);
    goto error_out;
  }

  field->next_field = next_field;

  return NULL;
error_out:
  return err;
}

cmc_error_t cmc_field_get_str(const struct cmc_ConfigField *field,
                              char **output) {
  if (field->value) {
    *output = field->value;
  } else if (field->optional) {
    *output = field->default_value;
  } else {
    return cmc_errorf(ENOENT,
                      "Field which is not optional, does not have value "
                      "`name=%s`. Did you parsed config?\n",
                      field->name);
  }

  return NULL;
};

cmc_error_t cmc_field_get_int(const struct cmc_ConfigField *field,
                              int32_t *output) {
  if (field->value) {
    *output = *(int32_t *)field->value;
  } else if (field->optional) {
    if (field->default_value) {
      *output = *(int32_t *)field->default_value;
    } else {
      *output = 0;
    }
  } else {
    return cmc_errorf(ENOENT,
                      "Field which is not optional, does not have value "
                      "`name=%s`. Did you parsed config?\n",
                      field->name);
  }

  return NULL;
};

void cmc_field_destroy(struct cmc_ConfigField **field) {
  if (!field || !*field) {
    return;
  }

  free((*field)->value);
  free((*field)->default_value);
  free((*field)->name);
  free(*field);
  *field = NULL;
};

cmc_error_t cmc_field_add_value_str(struct cmc_ConfigField *field,
                                    const char *value) {
  cmc_error_t err;
  if (!field || !value) {
    err = cmc_errorf(EINVAL, "`field=%p` and `value=%p` cannot be NULL\n",
                     field, value);
    goto error_out;
  }

  if (field->value) {
    err = cmc_errorf(
        EINVAL,
        "`field->value=%p` is already populated, cannot add `value=%p`\n",
        field->value, value);
    goto error_out;
  }

  err = cmc_alloc_field_value_str(value, &field->value);
  if (err) {
    goto error_out;
  }

  return NULL;

error_out:
  return err;
}

cmc_error_t cmc_field_add_value_int(struct cmc_ConfigField *field,
                                    const int32_t value) {
  cmc_error_t err;
  if (!field) {
    err = cmc_errorf(EINVAL, "`field=%p` cannot be NULL\n", field, value);
    goto error_out;
  }

  if (field->value) {
    err = cmc_errorf(
        EINVAL,
        "`field->value=%p` is already populated, cannot add `value=%p`\n",
        field->value, value);
    goto error_out;
  }

  err = cmc_alloc_field_value_int(value, &field->value);
  if (err) {
    goto error_out;
  }

  return NULL;

error_out:
  return err;
}

static inline cmc_error_t
cmc_field_add_default_value(struct cmc_ConfigField *field,
                            const void *default_value) {
  if (!default_value) {
    field->default_value = NULL;
    return NULL;
  }

  cmc_error_t err = NULL;
  switch (field->type) {
  case cmc_ConfigFieldTypeEnum_STRING:
    err = cmc_alloc_field_value_str(default_value, &field->default_value);
    break;
  case cmc_ConfigFieldTypeEnum_INT:
    err = cmc_alloc_field_value_int(*(int32_t *)default_value,
                                    &field->default_value);
    break;
  default:
    err = cmc_errorf(ENOMEM, "Unsupported value for `field->type=%d`\n",
                     field->type);
  }

  return err;
}

static inline cmc_error_t cmc_alloc_field_value_str(const char *value,
                                                    void **field_value) {
  if (!value || !field_value) {
    return cmc_errorf(EINVAL,
                      "`value=%p` and `field_value=%p` cannot be NULL\n");
  }

  *field_value = strdup(value);
  if (!*field_value) {
    return cmc_errorf(ENOMEM, "Unable to allocate memory for `field->value`\n");
  }

  return NULL;
}

static inline cmc_error_t cmc_alloc_field_value_int(const int32_t value,
                                                    void **field_value) {
  if (!field_value) {
    return cmc_errorf(EINVAL, "`field_value=%p` cannot be NULL\n");
  }

  int32_t *local_int = malloc(sizeof(int32_t));
  if (!local_int) {
    return cmc_errorf(ENOMEM, "Unable to allocate memory for `field->value`\n");
  }

  *local_int = value;
  *field_value = local_int;

  return NULL;
}
