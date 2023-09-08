# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) <2017-2023> Intel Corporation

if get_option('enable_lua')
	sources = files(
		'lua_config.c',
		'lua_dapi.c',
		'lua_dpdk.c',
		'lua_pktmbuf.c',
		'lua_socket.c',
		'lua_stdio.c',
		'lua_utils.c',
		'lua_vec.c')

	liblua = library('lua', sources,
		dependencies: [common, utils, vec, lua_dep, dpdk])
else
	sources = files('lua_config.c')
	liblua = library('lua', sources)
endif

lua = declare_dependency(link_with: liblua,
	include_directories: include_directories('.'))
