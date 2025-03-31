test_unity_name = 'test_unity.c'

unity_test_src = files([
  test_unity_name
])

test_unity_exe = executable('test_unity',
  sources: [
   unity_test_src,
   test_runner.process(test_unity_name)                   
  ],
  dependencies: [unity_dependency],
)

test('test_unity_installation', test_unity_exe, priority: 999)