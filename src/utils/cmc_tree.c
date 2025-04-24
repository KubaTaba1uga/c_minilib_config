/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#include <errno.h>
#include <stdlib.h>

#include "c_minilib_config.h"
#include "utils/cmc_error.h"
#include "utils/cmc_tree.h"

cmc_error_t cmc_tree_node_create(struct cmc_TreeNode *node) {
  cmc_error_t err;

  if (!node) {
    err = cmc_errorf(EINVAL, "`node=%p` cannot be NULL\n", node);
    goto error_out;
  }

  node->subnodes = NULL;
  node->subnodes_len = 0;

  return NULL;

error_out:
  return err;
}

void cmc_tree_node_destroy(struct cmc_TreeNode *node) {
  if (!node) {
    return;
  }

  free(node->subnodes);
  node->subnodes = NULL;
  node->subnodes_len = 0;
};

cmc_error_t cmc_tree_node_add_subnode(const struct cmc_TreeNode *subnode,
                                      struct cmc_TreeNode *node) {
  struct cmc_TreeNode **local_subnodes;
  cmc_error_t err;

  if (!node || !subnode) {
    err = cmc_errorf(EINVAL, "`node=%p` and `subnode=%p` cannot be NULL\n",
                     node, subnode);
    goto error_out;
  }

  local_subnodes = realloc(node->subnodes, (node->subnodes_len + 1) *
                                               sizeof(struct cmc_TreeNode *));
  if (!local_subnodes) {
    err =
        cmc_errorf(ENOMEM, "Unable to allocate moemory for `local_subnodes`\n");
    goto error_out;
  }

  node->subnodes = local_subnodes;
  node->subnodes[node->subnodes_len++] = (struct cmc_TreeNode *)subnode;

  return NULL;

error_out:
  return err;
}

cmc_error_t cmc_tree_node_pop_subnode(struct cmc_TreeNode *node) {
  struct cmc_TreeNode **local_subnodes;
  cmc_error_t err;

  if (!node) {
    err = cmc_errorf(EINVAL, "`node=%p` cannot be NULL\n", node);
    goto error_out;
  }

  if (node->subnodes_len <= 0) {
    err = cmc_errorf(EINVAL, "`node->subnodes_len=%s` cannot be 0\n",
                     node->subnodes_len);
    goto error_out;
  }

  local_subnodes = realloc(node->subnodes, (node->subnodes_len - 1) *
                                               sizeof(struct cmc_TreeNode *));
  if (!local_subnodes) {
    err =
        cmc_errorf(ENOMEM, "Unable to allocate moemory for `local_subnodes`\n");
    goto error_out;
  }

  node->subnodes = local_subnodes;
  node->subnodes_len--;

  return NULL;

error_out:
  return err;
};
