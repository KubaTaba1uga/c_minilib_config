/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#include <errno.h>
#include <stdlib.h>

#include "c_minilib_config.h"

#include "utils/cmc_tree.h"

cme_error_t cmc_tree_node_create(struct cmc_TreeNode *node) {
  cme_error_t err;

  if (!node) {
    err = cme_error(EINVAL, "`node` cannot be NULL");
    goto error_out;
  }

  node->subnodes = NULL;
  node->subnodes_len = 0;

  return NULL;

error_out:
  return cme_return(err);
}

void cmc_tree_node_destroy(struct cmc_TreeNode *node) {
  if (!node) {
    return;
  }

  free((void *)node->subnodes);
  node->subnodes = NULL;
  node->subnodes_len = 0;
};

cme_error_t cmc_tree_node_add_subnode(const struct cmc_TreeNode *subnode,
                                      struct cmc_TreeNode *node) {
  struct cmc_TreeNode **local_subnodes;
  cme_error_t err;

  if (!node || !subnode) {
    err = cme_error(EINVAL, "`node` and `subnode` cannot be NULL");
    goto error_out;
  }

  local_subnodes = (struct cmc_TreeNode **)realloc(
      (void *)node->subnodes,
      (node->subnodes_len + 1) * sizeof(struct cmc_TreeNode *));
  if (!local_subnodes) {
    err = cme_error(ENOMEM, "Unable to allocate moemory for `local_subnodes`");
    goto error_out;
  }

  node->subnodes = local_subnodes;
  node->subnodes[node->subnodes_len++] = (struct cmc_TreeNode *)subnode;

  return NULL;

error_out:
  return cme_return(err);
}

cme_error_t cmc_tree_node_pop_subnode(struct cmc_TreeNode *node) {
  struct cmc_TreeNode **local_subnodes;
  cme_error_t err;

  if (!node) {
    err = cme_error(EINVAL, "`node` cannot be NULL");
    goto error_out;
  }

  if (node->subnodes_len <= 0) {
    err = cme_error(EINVAL, "`node->subnodes_len` cannot be 0");
    goto error_out;
  }

  local_subnodes = (struct cmc_TreeNode **)realloc(
      (void *)node->subnodes,
      (node->subnodes_len - 1) * sizeof(struct cmc_TreeNode *));
  if (!local_subnodes) {
    err = cme_error(ENOMEM, "Unable to allocate moemory for `local_subnodes`");
    goto error_out;
  }

  node->subnodes = local_subnodes;
  node->subnodes_len--;

  return NULL;

error_out:
  return cme_return(err);
};
