#
#   Copyright(c) 2015-2016 Intel Corporation. All rights reserved.
#
#   SPDX-License-Indentifier: BSD-3-Clause

#

include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libpktgen_lua.a

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

CFLAGS += -O3 -g -DLUA_USE_LINUX -DUSE_64_BIT -DALLOW_EXPERIMENTAL_API
CFLAGS += -D_GNU_SOURCE $(WERROR_FLAGS) -I$(RTE_SRCDIR) -fno-stack-protector

LDLIBS += -lrte_eal -lrte_mempool -lrte_ethdev -lrte_mbuf -lrte_timer

ifeq ($(CONFIG_RTE_LIBRTE_VEC),y)
else
CFLAGS += -I$(SRCDIR)/../vec/$(RTE_TARGET)/include
endif

ifeq ($(CONFIG_RTE_LIBRTE_LUA),y)
else
PKG := $(shell which pkg-config)
ifeq (x$(PKG),x)
$(info "pkg-config command is not found using defaults")
$(info "Get lua-5.3 from lua.org and build it on your system")
$(info "Also install lua 5.3 into /usr/local/include and /usr/local/lib")
$(info "Make sure the library in /usr/local/lib is called liblua.a or liblua.so")
LUA_PKG_LIBS = "-L/usr/local/lib -llua"
LUA_PKG_CFLAGS = "-I/usr/local/include"
else
LUA_PKG_LIBS = $(shell pkg-config --silence-errors --libs lua5.3)
ifeq ($(LUA_PKG_LIBS),)
        LUA_PKG_LIBS = $(shell pkg-config --silence-errors --libs lua-5.3)
        LUA_PKG_CFLAGS = $(shell pkg-config --silence-errors --cflags lua-5.3)
endif
ifeq ($(LUA_PKG_LIBS),)
        LUA_PKG_LIBS = $(shell pkg-config --silence-errors --libs lua)
        LUA_PKG_CFLAGS = $(shell pkg-config --silence-errors --cflags lua)
endif
ifeq ($(LUA_PKG_LIBS),)
        $(info "Lua pkg-config was not found")
        $(info "Get lua-5.3 from lua.org and build it on your system")
        $(info "Also install lua 5.3 into /usr/local/include and /usr/local/lib")
        $(info "Make sure the library in /usr/local/lib is called liblua.a")
endif
LUA_PKG_LIBS = "-L/usr/local/lib -llua"
LUA_PKG_CFLAGS = "-I/usr/local/include"

CFLAGS += $(LUA_PKG_CFLAGS)
endif

CFLAGS += -I$(SRCDIR)/../lua/$(RTE_TARGET)/include
LDLIBS += -L$(SRCDIR)/../lua/$(RTE_TARGET)/lib  $(LUA_PKG_LIBS)
endif

ifeq ($(CONFIG_RTE_LIBRTE_UTILS),y)
else
CFLAGS += -I$(SRCDIR)/../utils/$(RTE_TARGET)/include
LDLIBS += -L$(SRCDIR)/../utils/$(RTE_TARGET)/lib -lutils
endif

LDLIBS += -pthread

# all source are stored in SRCS-y
SRCS-y := rte_lua.c rte_lua_stdio.c rte_lua_utils.c rte_lua_socket.c
SRCS-y += rte_lua_dpdk.c rte_lua_pktmbuf.c rte_lua_vec.c
SRCS-y += rte_lua_dapi.c

SYMLINK-y-include := rte_lua.h rte_lua_stdio.h rte_lua_utils.h rte_lua_socket.h
SYMLINK-y-include += rte_lua_dpdk.h rte_lua_pktmbuf.h rte_lua_vec.h
SYMLINK-y-include += rte_lua_dapi.h

include $(RTE_SDK)/mk/rte.extlib.mk
