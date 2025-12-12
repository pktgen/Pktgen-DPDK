sources = files(
	'copyright_info.c',
	'port_config.c',
	'cmdline_parse_args.c',
	'lscpu.c',
	'utils.c',
	'coreinfo.c',
	'pg_strings.c',
	'cksum.c',
	'pg_inet.c',
)
headers = files(
	'pg_inet.h',
	'port_config.h',
	'cmdline_parse_args.h',
	'lscpu.h',
	'utils.h',
	'coreinfo.h',
	'pg_strings.h',
	'cksum.h',
)
libcommon = library('common', sources, dependencies: [hmap, dpdk])
common = declare_dependency(link_with: libcommon, include_directories: include_directories('.'))
