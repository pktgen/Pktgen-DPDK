#   BSD LICENSE
#
#   Copyright(c) 2016-2017 Intel Corporation. All rights reserved. 
#   All rights reserved. 
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     * Neither the name of Intel Corporation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

include $(RTE_SDK)/$(RTE_TARGET)/.config
include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = librte_cli.a

CFLAGS += -O3 -D_GNU_SOURCE $(WERROR_FLAGS) -I$(SRCDIR)
CFLAGS += -DRTE_CLI_HOST_COMMANDS
CFLAGS += -DRTE_LIBRTE_LUA -DCLI_STANDALONE
CFLAGS += -I$(RTE_SRCDIR)/../lua/src


# all source are stored in SRCS-y
SRCS-y +=  cli.c
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
SRCS-y +=  cli_vt100_keys.c
SRCS-y +=  cli_scrn.c
SRCS-y +=  cli_string_fns.c

CFLAGS += -D_GNU_SOURCE

# install includes
SYMLINK-y-include += cli.h
SYMLINK-y-include += cli_common.h
SYMLINK-y-include += cli_map.h
SYMLINK-y-include += cli_gapbuf.h
SYMLINK-y-include += cli_file.h
SYMLINK-y-include += cli_env.h
SYMLINK-y-include += cli_search.h
SYMLINK-y-include += cli_help.h
SYMLINK-y-include += cli_history.h
SYMLINK-y-include += cli_vt100.h
SYMLINK-y-include += cli_vt100_keys.h
SYMLINK-y-include += cli_scrn.h
SYMLINK-y-include += cli_string_fns.h

# this lib needs eal
DEPDIRS-y += lib/librte_eal lib/librte_mempool
DEPDIRS-y += lib/librte_ether

include $(RTE_SDK)/mk/rte.extlib.mk
