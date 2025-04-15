unity_subproject = subproject('unity')

unity_dependency = unity_subproject.get_variable('unity_dep')

test_dependencies = [unity_dependency]

test_runner = unity_subproject.get_variable('gen_test_runner')

subdir('test_unity.d')
