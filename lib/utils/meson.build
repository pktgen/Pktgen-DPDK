# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2019 Intel Corporation

version = 1
allow_experimental_apis = true
headers = files(
	'_atoip.h',
	'link.h',
	'portlist.h',
	'heap.h')

sources = files(
	'_atoip.c',
	'link.c',
	'portlist.c',
	'heap.c')
deps += ['common']

libs = ['eal', 'ethdev', 'mempool']
foreach lib:libs
	ext_deps += cc.find_library('rte_' + lib, required: true, dirs: [dpdk_libdir])
endforeach
