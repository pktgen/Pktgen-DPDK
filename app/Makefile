#   BSD LICENSE
#
#   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
#
#   SPDX-License-Identifier: BSD-3-Clause
#
# Created 2010-2018 by Keith Wiles @ intel.com

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linux-gcc

include $(RTE_SDK)/mk/rte.vars.mk

# binary name
APP = pktgen

VER := $(shell cat $(RTE_SRCDIR)/../VERSION | \
	sed -e 's/-rc/.-rc/' -e 's/$$/..99/' | \
	awk -F '.' '{print "-D__YEAR="int($$1), "-D__MONTH="int($$2), "-D__MINOR="int($$3), "-D__SUFFIX=\\\""$$4"\\\"", "-D__RELEASE="int($$5)}')
CFLAGS += -O3 -g $(WERROR_FLAGS) -fno-stack-protector
CFLAGS += -I$(RTE_SRCDIR) -D_GNU_SOURCE
CFLAGS += -DALLOW_EXPERIMENTAL_API $(VER)

SRCS-y := \
	cli-functions.c lpktgenlib.c pktgen-cmds.c pktgen.c pktgen-cfg.c \
	pktgen-main.c pktgen-pcap.c pktgen-range.c pktgen-cpu.c \
	pktgen-seq.c pktgen-dump.c pktgen-capture.c pktgen-stats.c \
	pktgen-port-cfg.c pktgen-ipv6.c pktgen-ipv4.c pktgen-arp.c \
	pktgen-gre.c pktgen-ether.c pktgen-tcp.c pktgen-udp.c pktgen-vlan.c \
	pktgen-random.c pktgen-display.c pktgen-log.c pktgen-gtpu.c \
	pktgen-latency.c pktgen-rate.c


dpdk_ver   := $(RTE_SDK)/xusertools/dpdk-version.sh
pktgen_ver := $(RTE_SRCDIR)/../tools/dpdk-version.sh
ver_cmd    := $(if $(wildcard $(dpdk_ver)),$(dpdk_ver),$(pktgen_ver))
yy         := $(shell $(ver_cmd) -yy)

# $(info yy=$(yy))

COMMON_INC := -I$(RTE_SRCDIR)/../lib/common
LUA_INC    := -I$(RTE_SRCDIR)/../lib/lua
CLI_INC    := -I$(RTE_SRCDIR)/../lib/cli
UTILS_INC  := -I$(RTE_SRCDIR)/../lib/utils
VEC_INC    := -I$(RTE_SRCDIR)/../lib/vec
GUI_INC    := -I$(RTE_SRCDIR)/../gui/gui
PLUGIN_INC := -I$(RTE_SRCDIR)/../lib/plugin

COMMON_LIBDIR := $(RTE_SRCDIR)/../lib/common/$(RTE_TARGET)/lib
LUA_LIBDIR    := $(RTE_SRCDIR)/../lib/lua/$(RTE_TARGET)/lib
CLI_LIBDIR    := $(RTE_SRCDIR)/../lib/cli/$(RTE_TARGET)/lib
UTILS_LIBDIR  := $(RTE_SRCDIR)/../lib/utils/$(RTE_TARGET)/lib
VEC_LIBDIR    := $(RTE_SRCDIR)/../lib/vec/$(RTE_TARGET)/lib
GUI_LIBDIR    := $(RTE_SRCDIR)/../gui/$(RTE_TARGET)/lib
PLUGIN_LIBDIR := $(RTE_SRCDIR)/../lib/plugin/$(RTE_TARGET)/lib

COMMON_LIB := -L$(COMMON_LIBDIR)
LUA_LIB    := -L$(LUA_LIBDIR)
CLI_LIB    := -L$(CLI_LIBDIR)
UTILS_LIB  := -L$(UTILS_LIBDIR)
VEC_LIB    := -L$(VEC_LIBDIR)
GUI_LIB    := -L$(GUI_LIBDIR)
PLUGIN_LIB := -L$(PLUGIN_LIBDIR)

COMMON_LD  := -lcommon
LUA_LD     := -lpktgen_lua
CLI_LD     := -lcli
UTILS_LD   := -lutils
VEC_LD     := -lvec
GUI_LD     := -lgui
PLUGIN_LD  := -lplugin

# default vars to using common information
__CFLAGS   := $(COMMON_INC) $(PLUGIN_INC)
__LIBS     := $(COMMON_LIB) $(PLUGIN_LIB)
__LDFLAGS  := $(COMMON_LD) $(PLUGIN_LD)

__LIB_DEPS := $(COMMON_LIBDIR)/libcommon.a
__LIB_DEPS += $(PLUGIN_LIBDIR)/libplugin.a
__LIB_DEPS += $(CLI_LIBDIR)/libcli.a

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
LUA_PKG_LDFLAGS = -L/usr/local/lib
LUA_PKG_LIBS    = -llua
else
LUA_PKG_CFLAGS  = $(shell pkg-config --silence-errors --cflags lua-5.3)
LUA_PKG_LDFLAGS = $(shell pkg-config --silence-errors --libs-only-L lua-5.3)
LUA_PKG_LIBS    = $(shell pkg-config --silence-errors --libs-only-l lua-5.3)

ifeq ($(LUA_PKG_LIBS),)
LUA_PKG_CFLAGS  = $(shell pkg-config --silence-errors --cflags lua5.3)
LUA_PKG_LDFLAGS = $(shell pkg-config --silence-errors --libs-only-L lua5.3)
LUA_PKG_LIBS    = $(shell pkg-config --silence-errors --libs-only-l lua5.3)
endif

ifeq ($(LUA_PKG_LIBS),)
LUA_PKG_CFLAGS  = $(shell pkg-config --silence-errors --cflags lua)
LUA_PKG_LDFLAGS = $(shell pkg-config --silence-errors --libs-only-L lua)
LUA_PKG_LIBS    = $(shell pkg-config --silence-errors --libs-only-l lua)
endif

ifeq ($(LUA_PKG_LIBS),)
$(info "Lua pkg-config was not found")
$(info "Get lua-5.3 from lua.org and build it on your system")
$(info "Also install lua 5.3 into /usr/local/include and /usr/local/lib")
$(info "Make sure the library in /usr/local/lib is called liblua.a")

# set defaults
LUA_PKG_CFLAGS  = -I/usr/local/include
LUA_PKG_LDFLAGS = -L/usr/local/lib
LUA_PKG_LIBS    = -llua
endif
endif

# Pktgen Lua support code locations
__CFLAGS  += $(LUA_INC)
__LIBS    += $(LUA_LIB)
__LDFLAGS += $(LUA_LD)

__CFLAGS  += $(LUA_PKG_CFLAGS)
__LIBS    += $(LUA_PKG_LIBS)
__LDFLAGS += $(LUA_PKG_LDFLAGS)
endif

ifeq ($(CONFIG_RTE_LIBRTE_CLI),y)
else
__CFLAGS   += $(CLI_INC)
__LIBS     += $(CLI_LIB)
__LDFLAGS  += $(CLI_LD)
__LIB_DEPS += $(CLI_LIBDIR)/libcli.a
endif

ifeq ($(CONFIG_RTE_LIBRTE_UTILS),y)
else
__CFLAGS   += $(UTILS_INC)
__LIBS     += $(UTILS_LIB)
__LDFLAGS  += $(UTILS_LD)
__LIB_DEPS += $(UTILS_LIBDIR)/libutils.a
endif

ifeq ($(CONFIG_RTE_LIBRTE_VEC),y)
else
__CFLAGS   += $(VEC_INC)
__LIBS     += $(VEC_LIB)
__LDFLAGS  += $(VEC_LD)
__LIB_DEPS += $(VEC_LIBDIR)/libvec.a
endif

# GUI support is a work in progress
ifeq ($(GUI),true)
	__CFLAGS += -DGUI $(GUI_INC)

	__LIBS += -L$(GUI_LIB) -L/usr/lib64/gtk-2.0 -lgui \
		$(shell pkg-config --libs glib-2.0) \
		$(shell pkg-config --libs gtk+-2.0)
	DEPDIRS-y += ../gui
endif

ifeq ($(CONFIG_RTE_BUILD_SHARED_LIB),y)
	__LIBS += -lrte_pmd_ixgbe
endif

CFLAGS  += $(__CFLAGS)
LDFLAGS += --start-group $(__LDFLAGS) $(__LIBS) -lrte_timer --end-group -lpcap -lm

DEP_pktgen := $(__LIB_DEPS)

#$(info __CLFAGS = $(__CFLAGS))
#$(info __LIBS = $(__LIBS))
#$(info __LDFLAGS = $(__LDFLAGS))
#$(info __LIB_DEPS =  $(__LIB_DEPS))

include $(RTE_SDK)/mk/rte.extapp.mk
