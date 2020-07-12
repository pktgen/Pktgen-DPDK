project('pktgen', 'C',
    version: run_command(find_program('cat', 'more'),
        files('VERSION')).stdout().strip(),

	license: 'BSD',
	default_options: [
		'buildtype=release',
		'default_library=static',
		'warning_level=3',
		'werror=true'
	],
	meson_version: '>= 0.47.1'
)

pktgen_conf = configuration_data()

# set up some global vars for compiler, platform, configuration, etc.
cc = meson.get_compiler('c')

add_project_arguments('-mavx', language: 'c')
add_project_arguments('-DALLOW_EXPERIMENTAL_API', language: 'c')
add_project_arguments('-D_GNU_SOURCE', language: 'c')
if cc.has_argument('-Wno-format-truncation')
    add_project_arguments('-Wno-format-truncation', language: 'c')
endif
if get_option('enable_lua')
    add_project_arguments('-DLUA_ENABLED', language: 'c')
endif

dpdk = dependency('libdpdk', required: true)

subdir('tools')

subdir('lib')

subdir('app')

subdir('doc')
