# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) <2020-2024> Intel Corporation

sources = files(
	'_atoip.c',
	'portlist.c',
	'heap.c')
libutils = library('utils', sources,
	dependencies: [common, dpdk])
utils = declare_dependency(link_with: libutils,
	include_directories: include_directories('.'))
