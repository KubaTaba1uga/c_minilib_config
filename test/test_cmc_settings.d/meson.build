test_settings_name = 'test_cmc_settings.c'

settings_test_src = files([
  test_settings_name, '../../src/utils/cmc_settings.c'
])

test_settings_exe = executable('test_cmc_settings',
  sources: [
    settings_test_src,
    test_runner.process(test_settings_name),
  ],
  dependencies: test_dependencies,
  include_directories: test_includes,  
)

test('test_cmc_settings', test_settings_exe)
