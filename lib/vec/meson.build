# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) <2020-2023> Intel Corporation

sources = files('vec.c')
libvec = library('vec', sources,
	dependencies: [utils, dpdk])
vec = declare_dependency(link_with: libvec,
	include_directories: include_directories('.'))
