sources = files(
	'cli-functions.c', 'lpktgenlib.c', 'pktgen-cmds.c', 'pktgen.c', 'pktgen-cfg.c',
	'pktgen-main.c', 'pktgen-pcap.c', 'pktgen-range.c', 'pktgen-cpu.c',
	'pktgen-seq.c', 'pktgen-dump.c', 'pktgen-capture.c', 'pktgen-stats.c',
	'pktgen-port-cfg.c', 'pktgen-ipv6.c', 'pktgen-ipv4.c', 'pktgen-arp.c',
	'pktgen-gre.c', 'pktgen-ether.c', 'pktgen-tcp.c', 'pktgen-udp.c', 'pktgen-vlan.c',
	'pktgen-random.c', 'pktgen-display.c', 'pktgen-log.c', 'pktgen-gtpu.c',
	'pktgen-latency.c'
)

#TODO fixme - assumes previously build lua using make
lua_dir = '../lib/lua/x86_64-native-linuxapp-gcc'
lua_inc = include_directories(join_paths(lua_dir, 'include'))
lua = declare_dependency(include_directories: lua_inc, link_args: ['-L' + lua_dir, '-lrte_lua'])

pktgen = executable('dpdk-pktgen',
		sources,
		c_args: '-Wno-format-truncation',
		dependencies: [dep_cli, lua,
			dependency('threads'),
			cc.find_library('dl', required: false),
			cc.find_library('m', required: false)],
		install: true)

