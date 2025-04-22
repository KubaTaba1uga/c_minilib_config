# File: test/test_cmc_env_parser.d/meson.build
configs = {
  'array': '-DARRAY_CONFIG_PATH="@0@/array"',
  'array_nested': '-DARRAY_NESTED_CONFIG_PATH="@0@/array_nested"',
  'dict': '-DDICT_CONFIG_PATH="@0@/dict"',
  'kea': '-DKEA_CONFIG_PATH="@0@/kea"',
  'default': '-DCONFIG_PATH="@0@/config"',
}

foreach cfg_name, define_arg : configs
  full_define = define_arg.format(meson.current_source_dir())

  source_file = 'test_cmc_env_parser_' + cfg_name + '.c'
  exe_name = 'test_cmc_env_parser_' + cfg_name

  exe = executable(
    exe_name,
    sources: [
      source_file,
      test_runner.process(source_file),
    ],
    dependencies: test_dependencies,
    include_directories: test_includes,
    c_args: [full_define],
  )

  test(exe_name, exe)
endforeach

test_cmc_env_parser_name = 'test_cmc_env_parser.c'
