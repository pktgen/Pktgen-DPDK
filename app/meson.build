sources = files(
	'cli-functions.c', 'pktgen-cmds.c', 'pktgen.c', 'pktgen-cfg.c',
	'pktgen-main.c', 'pktgen-pcap.c', 'pktgen-range.c', 'pktgen-cpu.c',
	'pktgen-seq.c', 'pktgen-dump.c', 'pktgen-capture.c', 'pktgen-stats.c',
	'pktgen-port-cfg.c', 'pktgen-ipv6.c', 'pktgen-ipv4.c', 'pktgen-arp.c',
	'pktgen-gre.c', 'pktgen-ether.c', 'pktgen-tcp.c', 'pktgen-udp.c', 'pktgen-vlan.c',
	'pktgen-random.c', 'pktgen-display.c', 'pktgen-log.c', 'pktgen-gtpu.c',
	'pktgen-latency.c', 'pktgen-rate.c',
)

if get_option('enable_lua')
    sources += files('lpktgenlib.c')
endif

if get_option('enable_gui')
    sources += files(
        'pktgen-gui.c',
        'pktgen-gui-misc.c',
        'pktgen-gui-streams.c',
    )
endif

cflags = ['-D__PROJECT_VERSION="' + meson.project_version() + '"']

deps = [dpdk, common, utils, vec, plugin, cli, lua]

if get_option('enable_gui')
    deps += dependency('glib-2.0', required: true)
    deps += dependency('gtk+-2.0', required: true)
    cflags += '-Wno-deprecated-declarations'
    cflags += '-DGUI'
endif

deps += [cc.find_library('rte_net_i40e', dirs: [dpdk_libs_path], required: false)]
deps += [cc.find_library('rte_net_ixgbe', dirs: [dpdk_libs_path], required: false)]
deps += [cc.find_library('rte_net_ice', dirs: [dpdk_libs_path], required: false)]
deps += [cc.find_library('rte_bus_vdev', dirs: [dpdk_libs_path], required: false)]

deps += [dependency('threads')]
deps += [dependency('numa', required: true)]
deps += [dependency('pcap', required: true)]
deps += [cc.find_library('dl', required: false)]
deps += [cc.find_library('m', required: false)]

pktgen = executable('pktgen',
		sources,
		c_args: cflags,
		install: true,
		dependencies: [deps, lua_dep, dpdk_bond])
