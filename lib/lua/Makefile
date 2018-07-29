#
#   Copyright(c) 2015-2016 Intel Corporation. All rights reserved.
#
#   SPDX-License-Indentifier: BSD-3-Clause

#

include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = librte_lua.a

EXPORT_MAP := lua_version.map
LIBABIVER := 1

# To overcome problems with FD6 more strict warnings gcc flags,
# the following ones were removed:
#  -Wstrict-prototypes
#  -Wmissing-prototypes
#  -Wold-style-definition
#  -Wmissing-declarations
#  -Wcast-qual
WERROR_FLAGS := -W -Wall
WERROR_FLAGS += -Wpointer-arith
WERROR_FLAGS += -Wcast-align
WERROR_FLAGS += -Wnested-externs
WERROR_FLAGS += -Wno-cast-align

CFLAGS += -O3 -g -DLUA_USE_LINUX -DUSE_64_BIT
CFLAGS += $(WERROR_FLAGS) -I$(RTE_SRCDIR) -fno-stack-protector
CFLAGS += $(shell pkg-config --cflags lua5.3)

# all source are stored in SRCS-y
SRCS-y := rte_lua.c rte_lua_stdio.c rte_lua_utils.c rte_lua_socket.c

SYMLINK-y-include := rte_lua.h rte_lua_stdio.h rte_lua_utils.h rte_lua_socket.h

include $(RTE_SDK)/mk/rte.extlib.mk
