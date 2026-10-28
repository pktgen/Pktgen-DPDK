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
	meson_version: '>= 0.60.0'
)

pktgen_conf = configuration_data()

# set up some global vars for compiler, platform, configuration, etc.
cc = meson.get_compiler('c')
pktgen_source_root = meson.current_source_dir()
pktgen_build_root = meson.current_build_dir()

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

fgen_dep = dependency('libfgen', required: false)

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
	subdir('docs')
else
	dpdk = dependency('libdpdk', required: true, method: 'pkg-config',
		fallback: ['dpdk', 'libdpdk_dep'],
		default_options: [
			'default_library=static',
			'tests=false',
			'disable_apps=*',
			'enable_docs=false',
			'werror=false',
		])

	# The static subproject dependency already includes its enabled drivers.
	dpdk_driver_deps = []
	if dpdk.type_name() == 'pkgconfig'
		dpdk_libs_path = dpdk.get_variable('libdir')
		message('DPDK lib path: ' + dpdk_libs_path)
		foreach driver: ['rte_net_bond', 'rte_net_i40e', 'rte_net_ixgbe', 'rte_net_ice', 'rte_bus_vdev']
			dpdk_driver_deps += cc.find_library(driver, dirs: [dpdk_libs_path], required: false)
		endforeach
	endif

	subdirs =['tools', 'lib', 'examples', 'app', 'docs']
	foreach d:subdirs
		subdir(d)
	endforeach
endif
