# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2018 Intel Corporation

include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libplugin.a

CFLAGS += -D_GNU_SOURCE
CFLAGS += $(WERROR_FLAGS) -I$(SRCDIR) -O3

CFLAGS_plugin.o := -Wno-format-nonliteral

LDLIBS += -lrte_eal -lrte_hash -ldl

EXPORT_MAP := plugin_version.map

LIBABIVER := 1

# all source are stored in SRCS-y
SRCS-y := plugin.c

# install includes
SYMLINK-y-include := plugin.h

include $(RTE_SDK)/mk/rte.extlib.mk
