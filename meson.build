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

add_project_arguments('-march=native', language: 'c')
add_project_arguments('-mavx', language: 'c')
add_project_arguments('-DALLOW_EXPERIMENTAL_API', language: 'c')
add_project_arguments('-D_GNU_SOURCE', language: 'c')

# enable extra warnings and disable any unwanted warnings
warning_flags = [
	'-Wno-pedantic',
	'-Wno-format-truncation',
]
foreach arg: warning_flags
	if cc.has_argument(arg)
		add_project_arguments(arg, language: 'c')
	endif
endforeach

if get_option('enable_lua')
	add_project_arguments('-DLUA_ENABLED', language: 'c')

	lua_names = ['lua-5.3', 'lua5.3', 'lua-5.4', 'lua5.4']
	foreach n:lua_names
		lua_dep = dependency(n, required: false)
		if not lua_dep.found()
			lua_dep = cc.find_library(n, required: false)
		endif
		if lua_dep.found()
			break
		endif
	endforeach
	if not lua_dep.found()
		error('unable to find Lua')
	endif
else
	lua_dep = dependency('', required: false)
endif

dpdk = dependency('libdpdk', required: true)

subdir('tools')

subdir('lib')

subdir('app')

subdir('doc')
