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

cc = meson.get_compiler('c')

# ******************************************************************************
# *    App
# ******************************************************************************
enable_backtrace = false
if get_option('backtrace')
  add_project_arguments(['-D_GNU_SOURCE', '-DCME_ENABLE_BACKTRACE', '-fno-pie', '-fno-omit-frame-pointer'], language: 'c')
  add_project_link_arguments(['-no-pie'], language: 'c')
  enable_backtrace = true
endif

# Enable posix functions
add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c')

subdir('src')
libdl = cc.find_library('dl', required: true)

c_minilib_error_sub = subproject('c_minilib_error',
  default_options: ['backtrace=' + enable_backtrace.to_string()]  
)
c_minilib_error_dep = dependency('c_minilib_error',
  fallback: ['c_minilib_error', 'c_minilib_error_dep'],
  required: true,
)


c_minilib_config_deps = [c_minilib_error_dep, libdl]
c_minilib_config_inc = include_directories('include', 'src')
c_minilib_config_lib = library('c_minilib_config',
                         sources,
			 include_directories: c_minilib_config_inc,
                         dependencies: c_minilib_config_deps,
)

c_minilib_config_dep = declare_dependency(
  link_with: c_minilib_config_lib,
  include_directories: c_minilib_config_inc,
  dependencies: c_minilib_config_deps,  
)


# ******************************************************************************
# *    Tests
# ******************************************************************************
if get_option('tests')
   subdir('test')
endif

# ******************************************************************************
# *    Examples
# ******************************************************************************
if get_option('examples')
   subdir('example')
endif
