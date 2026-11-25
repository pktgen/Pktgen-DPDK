sources = files('pktperf.c', 'parse.c', 'port.c', 'stats.c', 'utils.c')

cflags = []

deps = [dpdk, common, utils, fgen_dep] + dpdk_driver_deps

deps += [dependency('threads')]
deps += [dependency('numa', required: true)]
deps += [dependency('pcap', required: true)]
deps += [cc.find_library('dl', required: false)]
deps += [cc.find_library('m', required: false)]

pktperf = executable('pktperf',
		sources,
		c_args: cflags,
		install: true,
		dependencies: deps)
