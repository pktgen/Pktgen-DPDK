# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2019 Intel Corporation

include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libutils.a

CFLAGS += $(WERROR_FLAGS) -I$(SRCDIR) -O3 -D_GNU_SOURCE
CFLAGS += -DALLOW_EXPERIMENTAL_API

LDLIBS += -lrte_ethdev -lrte_eal

EXPORT_MAP := utils_version.map

LIBABIVER := 1

# all source are stored in SRCS-y
SRCS-y += rte_strings.c rte_link.c parson_json.c
SRCS-y += rte_atoip.c rte_portlist.c inet_pton.c
SRCS-y += rte_heap.c

# install includes
SYMLINK-y-include += rte_strings.h rte_link.h
SYMLINK-y-include += rte_atoip.h rte_portlist.h parson_json.h
SYMLINK-y-include += rte_heap.h

include $(RTE_SDK)/mk/rte.extlib.mk
