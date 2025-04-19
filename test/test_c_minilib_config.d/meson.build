test_c_minilib_config_name = 'test_c_minilib_config.c'

c_minilib_config_test_src = files([
  test_c_minilib_config_name, 
])

test_c_minilib_config_exe = executable('test_cmc_c_minilib_config',
  sources: [
    c_minilib_config_test_src,
    test_runner.process(test_c_minilib_config_name),
  ],
  dependencies: test_dependencies,
  include_directories: test_includes,
  c_args: ['-DCONFIG_DIR="' + meson.current_source_dir() + '"'],    
)

test('test_cmc_c_minilib_config', test_c_minilib_config_exe)
