sources = files(
	'copyright_info.c',
	'port_config.c',
	'core_info.c',
	'cmdline_parse_args.c',
	'lscpu.c',
	'utils.c',
	'coremap.c',
	'pg_pcap.c',
	'pg_strings.c',
	'cksum.c',
	'l2p.c'
)

headers = files(
	'cksum.h',
	'cmdline_parse_args.h',
	'pg_compat.h',
	'copyright_info.h',
	'core_info.h',
	'coremap.h',
	'pg_delay.h',
	'l2p.h',
	'lscpu.h',
	'mbuf.h',
	'pg_pcap.h',
	'pg_ether.h',
	'pg_inet.h',
	'port_config.h',
	'pg_strings.h',
	'utils.h'
)

libs = ['eal', 'kvargs', 'cmdline', 'process_info']
foreach lib:libs
	ext_deps += cc.find_library('rte_' + lib, required: true, dirs: [dpdk_libdir])
endforeach

#ext_deps += dependency('threads')
ext_deps += cc.find_library('bsd', required: true)
#ext_deps += cc.find_library('numa', required: true)
#ext_deps += cc.find_library('dl', required: true)
