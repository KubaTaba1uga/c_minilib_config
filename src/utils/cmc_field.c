/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_minilib_config.h"
#include "c_minilib_error.h"
#include "utils/cmc_field.h"
#include "utils/cmc_tree.h"

static inline cme_error_t cmc_alloc_field_value_str(const char *value,
                                                    void **field_value);
static inline cme_error_t cmc_alloc_field_value_int(const int32_t value,
                                                    void **field_value);
static void cmc_field_value_destroy(struct cmc_ConfigField **field);

cme_error_t cmc_field_create(const char *name,
                             const enum cmc_ConfigFieldTypeEnum type,
                             const void *default_value, const bool optional,
                             struct cmc_ConfigField **field) {
  struct cmc_ConfigField *local_field;
  cme_error_t err;

  if (!name || !field) {
    err = cme_error(EINVAL, "`name` and `field` cannot be NULL");
    goto error_out;
  }

  local_field = malloc(sizeof(struct cmc_ConfigField));
  if (!local_field) {
    err = cme_error(ENOMEM, "Unable to allocate memory for `local_field`");
    goto error_out;
  }

  local_field->name = strdup(name);
  if (!local_field->name) {
    err =
        cme_error(ENOMEM, "Unable to allocate memory for `local_field->name`");
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
      err = cme_errorf(EINVAL, "`type=%d` unrecognized", type);
      goto error_field_name_cleanup;
    }
  }
  if (err) {
    goto error_field_cleanup;
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
  return cme_return(err);
};

cme_error_t cmc_field_add_subfield(struct cmc_ConfigField *field,
                                   struct cmc_ConfigField *child_field) {
  cme_error_t err;

  if (!field || !child_field) {
    err = cme_errorf(EINVAL, "`field` and `child_field` cannot be NULL", field,
                     child_field);
    goto error_out;
  }

  switch (field->type) {
  case cmc_ConfigFieldTypeEnum_ARRAY:
  case cmc_ConfigFieldTypeEnum_DICT:
    break;
  default:
    err = cme_errorf(EINVAL, "`field->type=%d` cannot be used as container",
                     field->type);
    goto error_out;
  }

  err = cmc_tree_node_add_subnode(&child_field->_self, &field->_self);
  if (err) {
    goto error_out;
  }

  return NULL;
error_out:
  return cme_return(err);
}

cme_error_t cmc_field_get_str(const struct cmc_ConfigField *field,
                              char **output) {
  if (field->value) {
    *output = field->value;
  } else if (field->optional) {
    *output = NULL;
  } else {
    return cme_errorf(
        ENOENT,
        "Missing value in field `field->name=%s`. Did you parsed config?",
        field->name);
  }

  return NULL;
};

cme_error_t cmc_field_get_int(const struct cmc_ConfigField *field,
                              int32_t *output) {
  if (field->value) {
    *output = *(int32_t *)field->value;
  } else if (field->optional) {
    *output = 0;
  } else {
    return cme_errorf(
        ENOENT,
        "Missing value in field `field->name=%s`. Did you parsed config?",
        field->name);
  }

  return NULL;
};

void cmc_field_destroy(struct cmc_ConfigField **field) {
  if (!field || !*field) {
    return;
  }

  CMC_FOREACH_FIELD(subfield, *field, { cmc_field_destroy(&subfield); });

  cmc_tree_node_destroy(&(*field)->_self);
  cmc_field_value_destroy(field);
  free((*field)->name);
  free((*field));
  *field = NULL;
}

cme_error_t cmc_field_add_value_str(struct cmc_ConfigField *field,
                                    const char *value) {
  cme_error_t err;
  if (!field || !value) {
    err = cme_error(EINVAL, "`field` and `value` cannot be NULL");
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
  return cme_return(err);
}

cme_error_t cmc_field_add_value_int(struct cmc_ConfigField *field,
                                    const int32_t value) {
  cme_error_t err;
  if (!field) {
    err = cme_error(EINVAL, "`field` cannot be NULL");
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
  return cme_return(err);
}
struct cmc_ConfigField *cmc_field_of_node(struct cmc_TreeNode *node_ptr) {
  return cmc_container_of(node_ptr, struct cmc_ConfigField, _self);
};

static inline cme_error_t cmc_alloc_field_value_str(const char *value,
                                                    void **field_value) {
  cme_error_t err;
  if (!value || !field_value) {
    err = cme_error(EINVAL, "`value` and `field_value` cannot be NULL");
    goto error_out;
  }

  *field_value = strdup(value);
  if (!*field_value) {
    err = cme_error(ENOMEM, "Unable to allocate memory for `field->value`");
    goto error_out;
  }

  return NULL;

error_out:
  return cme_return(err);
}

static inline cme_error_t cmc_alloc_field_value_int(const int32_t value,
                                                    void **field_value) {
  cme_error_t err;
  if (!field_value) {
    err = cme_error(EINVAL, "`field_value` cannot be NULL");
    goto error_out;
  }

  int32_t *local_int = malloc(sizeof(int32_t));
  if (!local_int) {
    err = cme_error(ENOMEM, "Unable to allocate memory for `field->value`");
    goto error_out;
  }

  *local_int = value;
  *field_value = local_int;

  return NULL;
error_out:
  return cme_return(err);
}

static void cmc_field_value_destroy(struct cmc_ConfigField **field) {
  if (!field || !*field) {
    return;
  }

  struct cmc_ConfigField *ptr = *field;

  free(ptr->value); // Only free scalar value

  ptr->value = NULL;
}
