sources = files(
	'cli-functions.c', 'lpktgenlib.c', 'pktgen-cmds.c', 'pktgen.c', 'pktgen-cfg.c',
	'pktgen-main.c', 'pktgen-pcap.c', 'pktgen-range.c', 'pktgen-cpu.c',
	'pktgen-seq.c', 'pktgen-dump.c', 'pktgen-capture.c', 'pktgen-stats.c',
	'pktgen-port-cfg.c', 'pktgen-ipv6.c', 'pktgen-ipv4.c', 'pktgen-arp.c',
	'pktgen-gre.c', 'pktgen-ether.c', 'pktgen-tcp.c', 'pktgen-udp.c', 'pktgen-vlan.c',
	'pktgen-random.c', 'pktgen-display.c', 'pktgen-log.c', 'pktgen-gtpu.c',
	'pktgen-latency.c', 'pktgen-rate.c'
)

message('Process the app directory')

#TODO fixme - assumes previously build lua using make
lua_dir = '../lib/lua/build'
lua_inc = include_directories(join_paths(lua_dir, 'include'))
lua = declare_dependency(include_directories: lua_inc, link_args: ['-L' + lua_dir, '-llua5.3'])

cflags = machine_args
if cc.has_argument('-Wno-format-truncation')
	cflags += '-Wno-format-truncation'
endif
pktgen = executable('dpdk-pktgen',
		sources,
		c_args: cflags,
		dependencies: [lua,
			cc.find_library('pcap', required: true),
			get_variable('static_pktgen_cli'),
			get_variable('static_pktgen_common'),
			get_variable('static_pktgen_lua'),
			get_variable('static_pktgen_plugin'),
			get_variable('static_pktgen_utils'),
			get_variable('static_pktgen_vec'),
			dependency('threads'),
			cc.find_library('dl', required: false),
			cc.find_library('m', required: false)],
		install: true)
