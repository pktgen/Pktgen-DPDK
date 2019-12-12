# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2017 Intel Corporation

version = 1
allow_experimental_apis = true
sources = files(
	'lua_config.c',
	'lua_dapi.c',
	'lua_dpdk.c',
	'lua_pktmbuf.c',
	'lua_socket.c',
	'lua_stdio.c',
	'lua_utils.c',
	'lua_vec.c')
headers = files(
	'lua_dapi.h',
	'lua_dpdk.h',
	'lua_config.h',
	'lua_pktmbuf.h',
	'lua_socket.h',
	'lua_stdio.h',
	'lua_utils.h',
	'lua_vec.h')
deps += ['common', 'vec']

libs = ['eal', 'mbuf']
foreach lib:libs
	ext_deps += cc.find_library('rte_' + lib, required: true, dirs: [dpdk_libdir])
endforeach

ext_deps += cc.find_library('lua5.3', required: true)
ext_deps += cc.find_library('pthread', required: true)
ext_deps += cc.find_library('m', required: true)
