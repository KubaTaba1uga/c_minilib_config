project('example_app', 'c', version: '0.1.0', default_options: ['c_std=c11'])

c_minilib_config_proj = subproject(
  'c_minilib_config',
  default_options: ['c_std=c11']
)
c_minilib_config_dep = c_minilib_config_proj.get_variable('cmx_config_dep')
c_minilib_config_inc = c_minilib_config_proj.get_variable('cmx_config_inc')

executable('example',
  'main.c',
  dependencies: [c_minilib_config_dep],
  include_directories: c_minilib_config_inc,
)
