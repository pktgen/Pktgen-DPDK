sources = files(
	'cli-functions.c',
    'l2p.c',
	'pktgen-arp.c',
	'pktgen-capture.c',
	'pktgen-cmds.c',
	'pktgen-cpu.c',
	'pktgen-display.c',
	'pktgen-dump.c',
	'pktgen-ether.c',
	'pktgen-gre.c',
	'pktgen-gtpu.c',
	'pktgen-ipv4.c',
	'pktgen-ipv6.c',
	'pktgen-latency.c',
	'pktgen-log.c',
	'pktgen-main.c',
	'pktgen-pcap.c',
	'pktgen-port-cfg.c',
	'pktgen-random.c',
	'pktgen-range.c',
	'pktgen-seq.c',
	'pktgen-stats.c',
	'pktgen-sys.c',
	'pktgen-tcp.c',
	'pktgen-udp.c',
	'pktgen-vlan.c',
	'pktgen.c',
	'xorshift64star.c',
)

if get_option('enable_lua')
    sources += files('lpktgenlib.c')
endif

cflags = ['-D__PROJECT_VERSION="' + meson.project_version() + '"']

deps = [dpdk, common, utils, vec, plugin, cli, lua, hmap] + dpdk_driver_deps

if fgen_dep.found()
    deps += [fgen_dep]
endif

deps += [dependency('threads')]
deps += [dependency('numa', required: true)]
deps += [dependency('pcap', required: true)]
deps += [cc.find_library('dl', required: false)]
deps += [cc.find_library('m', required: false)]
deps += [cc.find_library('bsd', required: true)]

pktgen = executable('pktgen',
		sources,
		c_args: cflags,
		install: true,
		dependencies: [deps, lua_dep])
