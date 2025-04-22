#ifndef C_MINILIB_CONFIG_CMC_TREE_H
#define C_MINILIB_CONFIG_CMC_TREE_H

#include <stdint.h>

#define cmc_container_of(ptr, type, member)                                    \
  ((type *)((char *)(ptr)-offsetof(type, member)))

struct cmc_tree_node {
  struct cmc_tree_node *subnodes;
  uint32_t subnodes_len;
};

#endif // C_MINILIB_CONFIG_CMC_TREE_H
