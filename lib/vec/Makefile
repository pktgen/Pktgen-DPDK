# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) <2016-2019> Intel Corporation

#
# Created by: Keith Wiles @ Intel.com

include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libvec.a

CFLAGS += $(WERROR_FLAGS) -I$(SRCDIR) -O3 -D_GNU_SOURCE
CFLAGS += -I$(RTE_SDK)/$(RTE_TARGET)/include

LDLIBS += -lrte_eal -lrte_mempool

EXPORT_MAP := vec_version.map

LIBABIVER := 1

# all source are stored in SRCS-y
SRCS-y := rte_vec.c

# install includes
SYMLINK-y-include += rte_vec.h

include $(RTE_SDK)/mk/rte.extlib.mk
