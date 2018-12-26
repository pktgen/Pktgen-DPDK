# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) <2016-2019> Intel Corporation.
#

include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libcli.a

CFLAGS += -O3 $(WERROR_FLAGS) -I$(SRCDIR) -D_GNU_SOURCE
CFLAGS += -DALLOW_EXPERIMENTAL_API

ifeq ($(CONFIG_RTE_LIBRTE_LUA),y)
else
CFLAGS += -I$(RTE_SRCDIR)/../lua/src
endif
ifeq ($(CONFIG_RTE_LIBRTE_UTILS),y)
else
CFLAGS += -I$(RTE_SRCDIR)/../utils
endif

LDLIBS += -L$(RTE_SRCDIR)/../utils/$(RTE_TARGET)/lib -lrte_eal -lrte_mempool -lrte_mbuf -lrte_timer
ifeq ($(CONFIG_RTE_LIBRTE_UTILS),y)
else
LDLIBS += -lutils
endif

EXPORT_MAP := cli_version.map

LIBABIVER := 1

# all source are stored in SRCS-y
SRCS-y +=  cli.c
SRCS-y +=  cli_input.c
SRCS-y +=  cli_cmds.c
SRCS-y +=  cli_map.c
SRCS-y +=  cli_gapbuf.c
SRCS-y +=  cli_file.c
SRCS-y +=  cli_env.c
SRCS-y +=  cli_auto_complete.c
SRCS-y +=  cli_help.c
SRCS-y +=  cli_history.c
SRCS-y +=  cli_search.c
SRCS-y +=  cli_cmap.c
SRCS-y +=  cli_vt100.c
SRCS-y +=  cli_scrn.c

# install includes
SYMLINK-y-include += cli.h
SYMLINK-y-include += cli_input.h
SYMLINK-y-include += cli_common.h
SYMLINK-y-include += cli_map.h
SYMLINK-y-include += cli_gapbuf.h
SYMLINK-y-include += cli_file.h
SYMLINK-y-include += cli_env.h
SYMLINK-y-include += cli_search.h
SYMLINK-y-include += cli_help.h
SYMLINK-y-include += cli_history.h
SYMLINK-y-include += cli_scrn.h

# this lib needs eal
DEPDIRS-y += lib/librte_eal lib/librte_mempool
DEPDIRS-y += lib/librte_ether

include $(RTE_SDK)/mk/rte.extlib.mk
