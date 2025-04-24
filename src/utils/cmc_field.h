#ifndef C_MINILIB_CONFIG_CMC_FIELD_H
#define C_MINILIB_CONFIG_CMC_FIELD_H

#include "c_minilib_config.h"
#include "utils/cmc_error.h"
#include "utils/cmc_tree.h"

cmc_error_t cmc_field_create(const char *name,
                             const enum cmc_ConfigFieldTypeEnum type,
                             const void *default_value, const bool optional,
                             struct cmc_ConfigField **field);

cmc_error_t cmc_field_add_value_str(struct cmc_ConfigField *field,
                                    const char *value);

cmc_error_t cmc_field_add_value_int(struct cmc_ConfigField *field,
                                    const int32_t value);

void cmc_field_destroy(struct cmc_ConfigField **field);

static inline struct cmc_ConfigField *
cmc_field_of_node(struct cmc_TreeNode *node_ptr) {
  return cmc_container_of(node_ptr, struct cmc_ConfigField, _self);
};

#define CMC_FOREACH_FIELD(var, field, func)                                    \
  CMC_TREE_SUBNODES_FOREACH(__##var##subnode, (field)->_self) {                \
    struct cmc_ConfigField *(var) = cmc_field_of_node(__##var##subnode);       \
    func                                                                       \
  }

#define CMC_FOREACH_FIELD_ARRAY(var, type, field, func)                        \
  CMC_TREE_SUBNODES_FOREACH(__##var##subnode, (*field)->_self) {               \
    struct cmc_ConfigField *__##var##_subfield =                               \
        cmc_field_of_node(__##var##subnode);                                   \
    type var = __##var##_subfield->value;                                      \
    func                                                                       \
  }

#define CMC_FOREACH_FIELD_DICT(var, type, field, func)                         \
  CMC_TREE_SUBNODES_FOREACH(__##var##subnode, (*field)->_self) {               \
    struct cmc_ConfigField *__##var##_subfield =                               \
        cmc_field_of_node(__##var##subnode);                                   \
    char *var##_name = __##var##_subfield->name;                               \
    type var = __##var##_subfield->value;                                      \
    func                                                                       \
  }

#endif // C_MINILIB_CONFIG_CMC_FIELD_H
