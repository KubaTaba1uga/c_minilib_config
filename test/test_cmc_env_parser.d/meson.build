# File: test/test_cmc_env_parser.d/meson.build

test_cmc_env_parser_name = 'test_cmc_env_parser.c'
test_cmc_env_parser_exe = executable(
  'test_cmc_env_parser',
  sources: [
    test_cmc_env_parser_name,
    test_runner.process(test_cmc_env_parser_name),
  ],
  dependencies: test_dependencies,
  include_directories: test_includes,
  c_args: ['-DCONFIG_PATH="' + meson.current_source_dir() + '/config"',
           '-DARRAY_CONFIG_PATH="' + meson.current_source_dir() + '/array"'
	   ],  
)
test('test_cmc_env_parser', test_cmc_env_parser_exe)

