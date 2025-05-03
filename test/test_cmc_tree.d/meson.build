test_tree_name = 'test_cmc_tree.c'

test_tree_exe = executable('test_cmc_tree',
  sources: [
    test_tree_name,
    test_runner.process(test_tree_name),
  ],
  dependencies: test_dependencies,
  include_directories: test_includes,
  c_args: ['-DCME_IMPL'],
)

test('test_cmc_tree', test_tree_exe)
