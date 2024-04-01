sources = files('pktperf.c', 'parse.c', 'port.c', 'stats.c', 'utils.c')

cflags = []

deps = [dpdk, common, utils, fgen_dep]

deps += [cc.find_library('rte_net_i40e', dirs: [dpdk_libs_path], required: false)]
deps += [cc.find_library('rte_net_ixgbe', dirs: [dpdk_libs_path], required: false)]
deps += [cc.find_library('rte_net_ice', dirs: [dpdk_libs_path], required: false)]
deps += [cc.find_library('rte_bus_vdev', dirs: [dpdk_libs_path], required: false)]

deps += [dependency('threads')]
deps += [dependency('numa', required: true)]
deps += [dependency('pcap', required: true)]
deps += [cc.find_library('dl', required: false)]
deps += [cc.find_library('m', required: false)]

pktperf = executable('pktperf',
		sources,
		c_args: cflags,
		install: true,
		dependencies: [deps, dpdk_bond])
