sources = files(
	'cli-functions.c', 'lpktgenlib.c', 'pktgen-cmds.c', 'pktgen.c', 'pktgen-cfg.c',
	'pktgen-main.c', 'pktgen-pcap.c', 'pktgen-range.c', 'pktgen-cpu.c',
	'pktgen-seq.c', 'pktgen-dump.c', 'pktgen-capture.c', 'pktgen-stats.c',
	'pktgen-port-cfg.c', 'pktgen-ipv6.c', 'pktgen-ipv4.c', 'pktgen-arp.c',
	'pktgen-gre.c', 'pktgen-ether.c', 'pktgen-tcp.c', 'pktgen-udp.c', 'pktgen-vlan.c',
	'pktgen-random.c', 'pktgen-display.c', 'pktgen-log.c', 'pktgen-gtpu.c',
	'pktgen-latency.c', 'pktgen-rate.c'
)

if get_option('enable_gui')
    sources += files(
        'pktgen-gui.c',
        'pktgen-gui-misc.c',
        'pktgen-gui-streams.c',
    )
endif

message('Process the app directory')

lua_dir = '../lib/'
lua_inc = include_directories(join_paths(lua_dir, 'lua'))
lua = declare_dependency(include_directories: lua_inc, link_args: ['-L' + lua_dir, '-llua5.3'])

cflags = machine_args
if cc.has_argument('-Wno-format-truncation')
	cflags += '-Wno-format-truncation'
endif
if get_option('is_lua_enabled')
    cflags += '-DLUA_ENABLED'
endif

cflags += '-DALLOW_EXPERIMENTAL_API'

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
	cflags += '-D__SUFFIX=""'
	cflags += '-D__RELEASE=99'
endif

deps = [dpdk_dep]

deps += [get_variable('static_pktgen_common')]
deps += [get_variable('static_pktgen_lua')]
deps += [get_variable('static_pktgen_plugin')]
deps += [get_variable('static_pktgen_utils')]
deps += [get_variable('static_pktgen_vec')]
deps += [get_variable('static_pktgen_cli')]

if get_option('enable_gui')
    deps += dependency('glib-2.0', required: true)
    deps += dependency('gtk+-2.0', required: true)
    cflags += '-Wno-deprecated-declarations'
endif

deps += [dependency('threads')]
if get_option('is_lua_enabled')
	deps += [cc.find_library('lua', required: true)]
endif
deps += [cc.find_library('numa', required: true)]
deps += [cc.find_library('pcap', required: true)]
deps += [cc.find_library('dl', required: false)]
deps += [cc.find_library('m', required: false)]

pktgen = executable('pktgen',
		sources,
		c_args: [cflags, '-pthread'],
		install: true,
		dependencies: deps)
