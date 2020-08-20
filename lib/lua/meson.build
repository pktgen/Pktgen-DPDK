# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2017 Intel Corporation

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
	deps += ['common', 'vec']

	ext_deps += cc.find_library('lua5.3', required: true)
	ext_deps += cc.find_library('pthread', required: true)
else
	sources = files('lua_config.c')
endif
