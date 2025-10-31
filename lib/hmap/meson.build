# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2018-2024 Intel Corporation

sources = files('hmap.c')
headers = files('hmap.h')

libhmap = library('hmap', sources, dependencies: dpdk)
hmap = declare_dependency(link_with: libhmap, include_directories: include_directories('.'))
