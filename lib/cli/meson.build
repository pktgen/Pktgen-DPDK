# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) <2020-2023> Intel Corporation

sources = files(
	'cli_auto_complete.c',
	'cli.c',
	'cli_cmap.c',
	'cli_cmds.c',
	'cli_env.c',
	'cli_file.c',
	'cli_gapbuf.c',
	'cli_help.c',
	'cli_history.c',
	'cli_input.c',
	'cli_map.c',
	'cli_scrn.c',
	'cli_search.c',
	'cli_vt100.c')
libcli = library('cli', sources,
	dependencies: [common, utils, dpdk])
cli = declare_dependency(link_with: libcli,
	include_directories: include_directories('.'))
