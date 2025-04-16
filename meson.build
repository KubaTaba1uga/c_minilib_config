# C Minilib Config
# ******************************************************************************
# *    Project
# ******************************************************************************
project('c_minilib_config', 'c',
         version: '0.0.1',
	 meson_version: '>=1.0.1',
	 default_options: [
            'werror=true',
            'c_std=c11',
         ],
)

# ******************************************************************************
# *    App
# ******************************************************************************
if get_option('backtrace').enabled()
  add_project_arguments('-DCME_ENABLE_BACKTRACE', language: 'c')
endif

# Enable posix functions
add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c')

subdir('src')

cmx_error_sub = subproject('c_minilib_error')
cmx_error_dep = cmx_error_sub.get_variable('cmx_error_dep')

cmx_config_inc = include_directories('include', 'src')
cmx_config_lib = library('cmx_config',
                         sources,
			 include_directories: cmx_config_inc,
                         dependencies: [cmx_error_dep],
)
cmx_config_dep = declare_dependency(
  link_with: cmx_config_lib,
  include_directories: cmx_config_inc,
  dependencies: [cmx_error_dep],  
)


# ******************************************************************************
# *    Tests
# ******************************************************************************

subdir('test')