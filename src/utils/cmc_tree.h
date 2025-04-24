#ifndef C_MINILIB_CONFIG_CMC_TREE_H
#define C_MINILIB_CONFIG_CMC_TREE_H

#include <stddef.h> /* for offsetof() */
#include <stdint.h>

#include "c_minilib_config.h"
#include "utils/cmc_common.h"

#define cmc_container_of(ptr, type, member)                                    \
  ((type *)((char *)(ptr)-offsetof(type, member)))

cmc_error_t cmc_tree_node_create(struct cmc_TreeNode *node);
void cmc_tree_node_destroy(struct cmc_TreeNode *node);

cmc_error_t cmc_tree_node_add_subnode(const struct cmc_TreeNode *subnode,
                                      struct cmc_TreeNode *node);
cmc_error_t cmc_tree_node_pop_subnode(struct cmc_TreeNode *node);

#define CMC_TREE_SUBNODES_FOREACH(var, node)                                   \
  struct cmc_TreeNode *var;                                                    \
  CMC_FOREACH(var, (node).subnodes, (node).subnodes_len)

#endif // C_MINILIB_CONFIG_CMC_TREE_H
