# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2019 Intel Corporation

include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libutils.a

CFLAGS += $(WERROR_FLAGS) -I$(SRCDIR) -O3 -D_GNU_SOURCE
CFLAGS += -DALLOW_EXPERIMENTAL_API -I$(SRCDIR)/../common/$(RTE_TARGET)/include

LDLIBS += -lrte_ethdev -lrte_eal

EXPORT_MAP := utils_version.map

LIBABIVER := 1

# all source are stored in SRCS-y
SRCS-y += link.c parson_json.c
SRCS-y += _atoip.c portlist.c inet_pton.c
SRCS-y += heap.c

# install includes
SYMLINK-y-include += link.h
SYMLINK-y-include += _atoip.h portlist.h parson_json.h
SYMLINK-y-include += heap.h

include $(RTE_SDK)/mk/rte.extlib.mk
