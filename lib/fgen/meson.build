# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2021 Intel Corporation

sources = files('fgen.c', 'parse.c', 'unparse.c')

libfgen = library('ften', sources, dependencies: common)
fgen = declare_dependency(link_with: libfgen, include_directories: include_directories('.'))
