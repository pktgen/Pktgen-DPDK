sources = files(
	'cli-functions.c', 'lpktgenlib.c', 'pktgen-cmds.c', 'pktgen.c', 'pktgen-cfg.c',
	'pktgen-main.c', 'pktgen-pcap.c', 'pktgen-range.c', 'pktgen-cpu.c',
	'pktgen-seq.c', 'pktgen-dump.c', 'pktgen-capture.c', 'pktgen-stats.c',
	'pktgen-port-cfg.c', 'pktgen-ipv6.c', 'pktgen-ipv4.c', 'pktgen-arp.c',
	'pktgen-gre.c', 'pktgen-ether.c', 'pktgen-tcp.c', 'pktgen-udp.c', 'pktgen-vlan.c',
	'pktgen-random.c', 'pktgen-display.c', 'pktgen-log.c', 'pktgen-gtpu.c',
	'pktgen-latency.c', 'pktgen-rate.c'
)

message('*** dpdk_libdir=' + dpdk_libdir)
message('Process the app directory')

lua_dir = '../lib/'
lua_inc = include_directories(join_paths(lua_dir, 'lua'))
lua = declare_dependency(include_directories: lua_inc, link_args: ['-L' + lua_dir, '-llua5.3'])

cflags = machine_args
if cc.has_argument('-Wno-format-truncation')
	cflags += '-Wno-format-truncation'
endif

# set the major version, which might be used by drivers and libraries
# depending on the configuration options
pver = meson.project_version().split('.')
major_version = '@0@.@1@'.format(pver.get(0), pver.get(1))

cflags += '-D__YEAR=' + pver.get(0)
cflags += '-D__MONTH=' + pver.get(1)
if pver.get(2).contains('-rc')
	rc_ver = pver.get(2).split('-rc')
	cflags += '-D__MINOR=' + rc_ver.get(0)
	cflags += '-D__SUFFIX="-rc"'
	cflags += '-D__RELEASE=' + rc_ver.get(1)
else
	cflags += '-D__MINOR=' + pver.get(2)
	cflags += '-D__SUFFIX='
	cflags += '-D__RELEASE=99'
endif

deps = []

flib = ['ethdev', 'net', 'gro', 'gso', 'cmdline', 'metrics', 'meter', 'bus_pci', 'pmd_i40e', 'pmd_ixgbe', 'pmd_ice']
foreach lib:flib
	deps += [cc.find_library('rte_' + lib, required: true, dirs: [dpdk_libdir])]
endforeach

deps += [get_variable('static_pktgen_common')]
deps += [get_variable('static_pktgen_lua')]
deps += [get_variable('static_pktgen_plugin')]
deps += [get_variable('static_pktgen_utils')]
deps += [get_variable('static_pktgen_vec')]
deps += [get_variable('static_pktgen_cli')]

deps += [dependency('threads')]
deps += [cc.find_library('lua', required: true)]
deps += [cc.find_library('numa', required: true)]
deps += [cc.find_library('pcap', required: true)]
deps += [cc.find_library('dl', required: false)]
deps += [cc.find_library('m', required: false)]

pmd_subdir_opt = get_option('drivers_install_subdir')
driver_install_path = join_paths(get_option('libdir'), pmd_subdir_opt)

pktgen = executable('dpdk-pktgen',
		sources,
		c_args: [cflags, '-pthread'],
		install_rpath: join_paths(get_option('prefix'), driver_install_path),
		install: true,
		dependencies: deps)
