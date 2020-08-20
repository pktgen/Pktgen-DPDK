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
libcommon = library('common', sources,
	dependencies: dpdk)
common = declare_dependency(link_with: libcommon,
	include_directories: include_directories('.'))
