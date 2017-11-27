#   BSD LICENSE
#
#   Copyright(c) 2016-2017 Intel Corporation.
#   All rights reserved.
#
#  SPDX-License-Identifier: BSD-3-Clause
#

include $(RTE_SDK)/$(RTE_TARGET)/.config
include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = librte_cli.a

CFLAGS += -O3 -D_GNU_SOURCE $(WERROR_FLAGS) -I$(SRCDIR)
CFLAGS += -DRTE_CLI_HOST_COMMANDS
CFLAGS += -DRTE_LIBRTE_LUA -DCLI_STANDALONE
CFLAGS += -I$(RTE_SRCDIR)/../lua/src

EXPORT_MAP := rte_cli_version.map

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
SRCS-y +=  cli_string_fns.c

CFLAGS += -D_GNU_SOURCE

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
SYMLINK-y-include += cli_string_fns.h

# this lib needs eal
DEPDIRS-y += lib/librte_eal lib/librte_mempool
DEPDIRS-y += lib/librte_ether

include $(RTE_SDK)/mk/rte.extlib.mk
