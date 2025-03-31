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
# Enable sanitizers
# add_project_arguments('-fsanitize=address,undefined', language: 'c')

# Enable posix functions
add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c')

subdir('src')

main = library('cmx_config', sources, include_directories: ['include', 'src'])



# ******************************************************************************
# *    Tests
# ******************************************************************************

subdir('test')