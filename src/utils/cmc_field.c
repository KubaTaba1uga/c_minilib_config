#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_minilib_config.h"
#include "utils/cmc_field.h"
#include "utils/cmc_tree.h"

static inline cmc_error_t cmc_alloc_field_value_str(const char *value,
                                                    void **field_value);
static inline cmc_error_t cmc_alloc_field_value_int(const int32_t value,
                                                    void **field_value);
static void cmc_field_value_destroy(struct cmc_ConfigField **field);

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

  err = cmc_tree_node_create(&local_field->_self);
  if (err) {
    goto error_field_cleanup;
  }

  if (!optional) {
    default_value = NULL;
  }

  local_field->value = NULL;
  if (default_value) {
    switch (type) {
    case cmc_ConfigFieldTypeEnum_INT:
      err = cmc_field_add_value_int(local_field, *(int32_t *)default_value);
      break;
    case cmc_ConfigFieldTypeEnum_STRING:
      err = cmc_field_add_value_str(local_field, (char *)default_value);
      break;
    case cmc_ConfigFieldTypeEnum_DICT:
    case cmc_ConfigFieldTypeEnum_ARRAY:
      local_field->value = NULL;
      break;
    default:
      err = cmc_errorf(EINVAL, "`type=%d` unrecognized\n", type);
      goto error_field_name_cleanup;
    }
  }

  local_field->optional = optional;
  local_field->type = type;
  *field = local_field;

  return NULL;

error_field_name_cleanup:
  free(local_field->name);
error_field_cleanup:
  free(local_field);
error_out:
  return err;
};

cmc_error_t cmc_field_add_subfield(struct cmc_ConfigField *field,
                                   struct cmc_ConfigField *child_field) {
  cmc_error_t err;

  if (!field || !child_field) {
    err = cmc_errorf(EINVAL, "`field=%p` and `child_field=%p` cannot be NULL\n",
                     field, child_field);
    goto error_out;
  }

  switch (field->type) {
  case cmc_ConfigFieldTypeEnum_ARRAY:
  case cmc_ConfigFieldTypeEnum_DICT:
    break;
  default:
    err = cmc_errorf(EINVAL, "`field->type=%d` cannot be used as container\n",
                     field->type);
    goto error_out;
  }

  err = cmc_tree_node_add_subnode(&child_field->_self, &field->_self);
  if (err) {
    goto error_out;
  }

  printf("%s=%d\n", field->name, field->_self.subnodes_len);

  return NULL;
error_out:
  return err;
}

cmc_error_t cmc_field_get_str(const struct cmc_ConfigField *field,
                              char **output) {
  if (field->value) {
    *output = field->value;
  } else if (field->optional) {
    *output = NULL;
  } else {
    return cmc_errorf(
        ENOENT,
        "Missing value in field `field->name=%s`. Did you parsed config?\n",
        field->name);
  }

  return NULL;
};

cmc_error_t cmc_field_get_int(const struct cmc_ConfigField *field,
                              int32_t *output) {
  if (field->value) {
    *output = *(int32_t *)field->value;
  } else if (field->optional) {
    *output = 0;
  } else {
    return cmc_errorf(
        ENOENT,
        "Missing value in field `field->name=%s`. Did you parsed config?\n",
        field->name);
  }

  return NULL;
};

void cmc_field_destroy(struct cmc_ConfigField **field) {
  if (!field || !*field) {
    return;
  }

  CMC_FIELD_FOREACH(subfield, *field, { cmc_field_destroy(&subfield); });

  cmc_tree_node_destroy(&(*field)->_self);
  cmc_field_value_destroy(field);
  free((*field)->name);
  free((*field));
  *field = NULL;
}

cmc_error_t cmc_field_add_value_str(struct cmc_ConfigField *field,
                                    const char *value) {
  cmc_error_t err;
  if (!field || !value) {
    err = cmc_errorf(EINVAL, "`field=%p` and `value=%p` cannot be NULL\n",
                     field, value);
    goto error_out;
  }

  if (field->value) {
    free(field->value);
    field->value = NULL;
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
    free(field->value);
    field->value = NULL;
  }

  err = cmc_alloc_field_value_int(value, &field->value);
  if (err) {
    goto error_out;
  }

  return NULL;

error_out:
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

static void cmc_field_value_destroy(struct cmc_ConfigField **field) {
  if (!field || !*field) {
    return;
  }

  struct cmc_ConfigField *ptr = *field;

  free(ptr->value); // Only free scalar value

  ptr->value = NULL;
}
