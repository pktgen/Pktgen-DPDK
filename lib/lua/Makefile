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
CFLAGS += -I$(SRCDIR)/../common/$(RTE_TARGET)/include

LDLIBS += -lrte_eal -lrte_mempool -lrte_ethdev -lrte_mbuf -lrte_timer
LDLIBS += -L../utils/$(RTE_TARGET)/lib -lutils

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

# set defaults
LUA_PKG_CFLAGS  = -I/usr/local/include
LUA_PKG_LIBS    = -L/usr/local/lib -llua
else
LUA_PKG_CFLAGS  = $(shell pkg-config --silence-errors --cflags lua-5.3)
LUA_PKG_LIBS    = $(shell pkg-config --silence-errors --libs lua-5.3)

ifeq ($(LUA_PKG_LIBS),)
LUA_PKG_CFLAGS  = $(shell pkg-config --silence-errors --cflags lua5.3)
LUA_PKG_LIBS     = $(shell pkg-config --silence-errors --libs lua5.3)
endif

ifeq ($(LUA_PKG_LIBS),)
LUA_PKG_CFLAGS  = $(shell pkg-config --silence-errors --cflags lua)
LUA_PKG_LIBS    = $(shell pkg-config --silence-errors --libs lua)
endif

ifeq ($(LUA_PKG_LIBS),)
$(info "Lua pkg-config file was not found")
$(info "Get lua-5.3 from lua.org and build it on your system")
$(info "Also install lua 5.3 into /usr/local/include and /usr/local/lib")
$(info "Make sure the library in /usr/local/lib is called liblua.a")

# set defaults
LUA_PKG_CFLAGS  = -I/usr/local/include
LUA_PKG_LIBS    = -L/usr/local/lib -llua
endif
endif

# Pktgen Lua support code locations
CFLAGS += -I$(RTE_SRCDIR)/../lua/$(RTE_TARGET)/include $(LUA_PKG_CFLAGS)
LDLIBS += $(LUA_PKG_LIBS)
endif

ifeq ($(CONFIG_RTE_LIBRTE_UTILS),y)
else
CFLAGS  += -I$(SRCDIR)/../utils/$(RTE_TARGET)/include
endif

LDLIBS += -pthread

# all source are stored in SRCS-y
SRCS-y := lua_config.c lua_stdio.c lua_utils.c lua_socket.c
SRCS-y += lua_dpdk.c lua_pktmbuf.c lua_vec.c
SRCS-y += lua_dapi.c

SYMLINK-y-include := lua_config.h lua_stdio.h lua_utils.h lua_socket.h
SYMLINK-y-include += lua_dpdk.h lua_pktmbuf.h lua_vec.h
SYMLINK-y-include += lua_dapi.h

include $(RTE_SDK)/mk/rte.extlib.mk
