test_cmc_field_name = 'test_cmc_field.c'

test_cmc_field_exe = executable(
  'test_cmc_field',
  sources: [
    test_cmc_field_name,
    test_runner.process(test_cmc_field_name),
  ],
  dependencies: test_dependencies,
  include_directories: test_includes,
)

test('test_cmc_field', test_cmc_field_exe)
