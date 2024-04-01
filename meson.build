project('pktgen', 'C',
    version: run_command(find_program('cat', 'more'),
        files('VERSION'), check:false).stdout().strip(),

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

target = target_machine.cpu_family()
if (target != 'riscv64') and (target != 'aarch64')
    add_project_arguments('-march=native', language: 'c')
endif

if get_option('enable-avx') and cc.has_argument('-mavx')
    add_project_arguments('-mavx', language: 'c')
endif
if get_option('enable-avx2') and cc.has_argument('-mavx2')
	add_project_arguments('-mavx2', language: 'c')
endif
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

fgen_dep = dependency('libfgen', required: true)

lua_dep = dependency('', required: false)

if get_option('enable_lua')
	message('>>>>>>>>>>>>> Lua enabled <<<<<<<<<<<<<<')
	add_project_arguments('-DLUA_ENABLED', language: 'c')

	lua_names = ['lua', 'lua-5.3', 'lua5.3', 'lua-5.4', 'lua5.4']
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
endif

if get_option('only_docs')
	subdir('tools')
	subdir('doc')
else
	dpdk = dependency('libdpdk', required: true, method: 'pkg-config')
	dpdk_prefix = dpdk.get_pkgconfig_variable('prefix')
	message('prefix: ' + dpdk_prefix + ' libdir: ' + get_option('libdir'))
	dpdk_libs_path = join_paths(dpdk_prefix, get_option('libdir'))
	message('DPDK lib path: ' + dpdk_libs_path)

	dpdk_bond = cc.find_library('rte_net_bond', dirs: [dpdk_libs_path], required: false)

	subdirs =['tools', 'lib', 'examples', 'app', 'doc']
	foreach d:subdirs
		subdir(d)
	endforeach
endif
