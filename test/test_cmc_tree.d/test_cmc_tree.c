#include "utils/cmc_error.h"
#include "utils/cmc_tree.h"
#include <unity.h>

static struct cmc_TreeNode node;
static struct cmc_TreeNode child;
static cmc_error_t err = NULL;

void setUp(void) {
  err = cmc_tree_node_create(&node);
  TEST_ASSERT_NULL(err);
  err = cmc_tree_node_create(&child);
  TEST_ASSERT_NULL(err);
}

void tearDown(void) {
  cmc_error_destroy(&err);
  cmc_tree_node_destroy(&node);
}

void test_create_node_sets_defaults(void) {
  TEST_ASSERT_NULL(node.subnodes);
  TEST_ASSERT_EQUAL_UINT32(0, node.subnodes_len);
}

void test_add_subnode_increments_length(void) {
  err = cmc_tree_node_add_subnode(&child, &node);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_UINT32(1, node.subnodes_len);
  TEST_ASSERT_EQUAL_PTR(&child, node.subnodes[0]);
}

void test_add_multiple_subnodes(void) {
  struct cmc_TreeNode c1, c2, c3;
  cmc_tree_node_create(&c1);
  cmc_tree_node_create(&c2);
  cmc_tree_node_create(&c3);

  err = cmc_tree_node_add_subnode(&c1, &node);
  TEST_ASSERT_NULL(err);
  err = cmc_tree_node_add_subnode(&c2, &node);
  TEST_ASSERT_NULL(err);
  err = cmc_tree_node_add_subnode(&c3, &node);
  TEST_ASSERT_NULL(err);

  TEST_ASSERT_EQUAL_UINT32(3, node.subnodes_len);
  TEST_ASSERT_EQUAL_PTR(&c3, node.subnodes[2]);
}

void test_null_input_handling(void) {
  err = cmc_tree_node_add_subnode(NULL, &node);
  TEST_ASSERT_NOT_NULL(err);
  cmc_error_destroy(&err);

  err = cmc_tree_node_add_subnode(&child, NULL);
  TEST_ASSERT_NOT_NULL(err);
}
