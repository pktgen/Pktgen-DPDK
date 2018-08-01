# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2018 Intel Corporation

include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libutils.a

CFLAGS += $(WERROR_FLAGS) -I$(SRCDIR) -O3
CFLAGS += -DALLOW_EXPERIMENTAL_API

EXPORT_MAP := rte_utils_version.map

LIBABIVER := 1

# all source are stored in SRCS-y
SRCS-y += rte_str_fns.c rte_link.c rte_atoip.c rte_portlist.c

# install includes
SYMLINK-y-include += rte_utils.h rte_str_fns.h rte_link.h rte_atoip.h rte_portlist.h

include $(RTE_SDK)/mk/rte.extlib.mk
