#ifndef C_MINILIB_CONFIG_CMC_FIELD_H
#define C_MINILIB_CONFIG_CMC_FIELD_H

#include "c_minilib_config.h"
#include "utils/cmc_error.h"

cmc_error_t cmc_field_create(const char *name,
                             const enum cmc_ConfigFieldTypeEnum type,
                             const void *default_value, const bool optional,
                             struct cmc_ConfigField **field);

cmc_error_t cmc_field_add_value_str(struct cmc_ConfigField *field,
                                    const char *value);

cmc_error_t cmc_field_add_value_int(struct cmc_ConfigField *field,
                                    const int32_t value);

void cmc_field_destroy(struct cmc_ConfigField **field);

#define CMC_FIELD_ITER(var, field)                                             \
  for (struct cmc_ConfigField *var = (field); (var) != NULL;                   \
       (var) = (var)->next_field)

static inline struct cmc_ConfigField *cmc_field_of_node(struct cmc_tree_node *node_ptr){
  return cmc_container_of(node_ptr, struct cmc_ConfigField, self);
};
#endif // C_MINILIB_CONFIG_CMC_FIELD_H
