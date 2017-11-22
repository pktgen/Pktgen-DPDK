#   BSD LICENSE
#
#   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
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
#
# Created 2010-2017 by Keith Wiles @ intel.com

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

# binary name
APP = pktgen

CFLAGS += -O3 -g $(WERROR_FLAGS) -fno-stack-protector
CFLAGS += -I$(RTE_SRCDIR) -I$(RTE_SRCDIR)/../lib/common

dpdk_ver := $(RTE_SDK)/xusertools/dpdk-version.sh
pktgen_ver := $(RTE_SRCDIR)/../tools/dpdk-version.sh
ver_cmd := $(if $(wildcard $(dpdk_ver)),$(dpdk_ver),$(pktgen_ver))
# $(info ver_cmd=$(ver_cmd))

yy := $(shell $(ver_cmd) -yy)
# mm := $(shell $(ver_cmd) -mm)

# $(info yy=$(yy))

<<<<<<< HEAD
=======
ifeq ($(yy),17)
COMMON_PRE := $(RTE_OUTPUT)/../../lib/common
LUA_PRE := $(RTE_OUTPUT)/../../lib/lua/src
CLI_PRE := $(RTE_OUTPUT)/../../lib/cli
GUI_PRE := $(RTE_OUTPUT)/../../gui/gui
else
>>>>>>> master
ifeq ($(yy),16)
COMMON_PRE := $(RTE_SRCDIR)/../lib/common/lib/common
LUA_PRE := $(RTE_SRCDIR)/../lib/lua/src/lib/lua/src
CLI_PRE := $(RTE_SRCDIR)/../lib/cli/lib/cli
GUI_PRE := $(RTE_SRCDIR)/../gui
else
COMMON_PRE := $(RTE_SRCDIR)/../lib/common
LUA_PRE := $(RTE_SRCDIR)/../lib/lua/src
CLI_PRE := $(RTE_SRCDIR)/../lib/cli
GUI_PRE := $(RTE_SRCDIR)/../gui/gui
endif

COMMON_LIB := $(COMMON_PRE)/$(RTE_TARGET)/lib
LUA_LIB    := $(LUA_PRE)/$(RTE_TARGET)/lib
CLI_LIB    := $(CLI_PRE)/$(RTE_TARGET)/lib
GUI_LIB    := $(GUI_PRE)/$(RTE_TARGET)/lib

SRCS-y := \
	cli-functions.c lpktgenlib.c pktgen-cmds.c pktgen.c pktgen-cfg.c \
	pktgen-main.c pktgen-pcap.c pktgen-range.c pktgen-cpu.c \
	pktgen-seq.c pktgen-dump.c pktgen-capture.c pktgen-stats.c \
	pktgen-port-cfg.c pktgen-ipv6.c pktgen-ipv4.c pktgen-arp.c \
	pktgen-gre.c pktgen-ether.c pktgen-tcp.c pktgen-udp.c pktgen-vlan.c \
	pktgen-random.c pktgen-display.c pktgen-log.c pktgen-gtpu.c \
	pktgen-latency.c

COMMON_NAME := libcommon.a
MYLIBS     += -lcommon
MYLIB_PATH += -L$(COMMON_LIB) 

LUA_NAME   := librte_lua.a
MYLIBS     += -lrte_lua
MYLIB_PATH += -L$(LUA_LIB) 

CLI_NAME   := librte_cli.a
MYLIBS     += -lrte_cli
MYLIB_PATH += -L$(CLI_LIB)

CFLAGS     += -I$(RTE_SRCDIR)/../lib/common
COMMON_DEP := $(COMMON_LIB)/$(COMMON_NAME)

ifeq ($(CONFIG_RTE_LIBRTE_CLI),)
CFLAGS     += -I$(RTE_SRCDIR)/../lib/cli
CLI_DEP    := $(CLI_LIB)/$(CLI_NAME)
endif

LUA_DEP    := $(LUA_LIB)/$(LUA_NAME)
CFLAGS     += -I$(RTE_SRCDIR)/../lib/lua/src

ifeq ($(CONFIG_RTE_BUILD_SHARED_LIB),y)
	MYLIBS += -lrte_pmd_ixgbe
ifeq ($(CONFIG_RTE_LIBRTE_LUA),)
	RPATHS += -rpath=$(LUA_LIB)
endif
	RPATHS += -rpath=$(COMMON_LIB)
endif

# GUI support is a work in progress
ifeq ($(GUI),true)
	CFLAGS += -DGUI

	MYLIB_PATH += -L$(GUI_LIB) -L/usr/lib64/gtk-2.0
	MYLIBS += -lgui \
		$(shell pkg-config --libs glib-2.0) \
		$(shell pkg-config --libs gtk+-2.0)

	ifeq ($(CONFIG_RTE_BUILD_SHARED_LIB),y)
		RPATHS += -rpath=$(GUI_LIB)
	endif

	DEPDIRS-y += ../gui
endif

# $(info $(CLI_LIB)/$(CLI_NAME))
# $(info $(MYLIB_PATH))

pktgen: $(COMMON_DEP) $(LUA_DEP) $(CLI_DEP)

LDFLAGS += -g -lpcap $(RPATHS) $(MYLIB_PATH) $(MYLIBS) -lm

include $(RTE_SDK)/mk/rte.extapp.mk
