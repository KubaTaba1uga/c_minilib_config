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

c_minilib_error_subproject = subproject('c_minilib_error')
cmx_config_lib = library('cmx_config',
                         sources,
			 include_directories: ['include', 'src'],
                         dependencies: [c_minilib_error_subproject.get_variable('cmx_error_dep')],
)


# ******************************************************************************
# *    Tests
# ******************************************************************************

subdir('test')