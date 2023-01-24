# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) <2018-2023> Intel Corporation

sources = files('plugin.c')
libplugin = library('plugin', sources,
	dependencies: [common, dpdk])
plugin = declare_dependency(link_with: libplugin,
	include_directories: include_directories('.'))
