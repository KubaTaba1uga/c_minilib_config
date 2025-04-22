# File: test/meson.build

unity_subproject = subproject('unity')

unity_dependency = unity_subproject.get_variable('unity_dep')

test_dependencies = [cmx_config_dep, unity_dependency]

test_includes = [cmx_config_inc]

test_runner = unity_subproject.get_variable('gen_test_runner')

subdir('test_unity.d')
subdir('test_cmc_settings.d')
subdir('test_cmc_tree.d')
subdir('test_cmc_field.d')
subdir('test_cmc_env_parser.d')
# subdir('test_c_minilib_config.d')
