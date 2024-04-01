sources = files(
	'copyright_info.c',
	'port_config.c',
	'cmdline_parse_args.c',
	'lscpu.c',
	'utils.c',
	'coreinfo.c',
	'pg_strings.c',
	'cksum.c',
)
libcommon = library('common', sources, dependencies: [hmap, dpdk])
common = declare_dependency(link_with: libcommon, include_directories: include_directories('.'))
