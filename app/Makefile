#
# Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
#
# 1) Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2) Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
#
# 3) Neither the name of Wind River Systems nor the names of its contributors may be
# used to endorse or promote products derived from this software without specific
# prior written permission.
#
# 4) The screens displayed by the application must contain the copyright notice as defined
# above and can not be removed without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Created 2010-2014 by Keith Wiles @ windriver.com

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

# binary name
APP = pktgen

CFLAGS_commands.o := -Wno-format-nonliteral
CFLAGS_pktgen-log.o := -Wno-format-nonliteral

CFLAGS += -O3 -g $(WERROR_FLAGS) -fno-stack-protector
CFLAGS += -I$(RTE_SRCDIR) -I$(RTE_SRCDIR)/../lib/common
CFLAGS += -I$(RTE_SRCDIR)/../lib/lua/src

LUA_LIB    := $(RTE_SRCDIR)/../lib/lua/src/lib/lua/src/$(RTE_TARGET)/lib
COMMON_LIB := $(RTE_SRCDIR)/../lib/common/lib/common/$(RTE_TARGET)/lib
GUI_LIB    := $(RTE_SRCDIR)/../gui/gui/$(RTE_TARGET)/lib

# all source are stored in SRCS-y
SRCS-y := \
	cmd-functions.c lpktgenlib.c pktgen-cmds.c pktgen.c \
	pktgen-main.c pktgen-pcap.c pktgen-range.c pktgen-cpu.c \
	pktgen-seq.c pktgen-dump.c pktgen-capture.c pktgen-stats.c \
	pktgen-port-cfg.c pktgen-ipv6.c pktgen-ipv4.c pktgen-arp.c \
	pktgen-gre.c pktgen-ether.c pktgen-tcp.c pktgen-udp.c pktgen-vlan.c \
	pktgen-random.c pktgen-display.c pktgen-log.c pktgen-gtpu.c \
	pktgen-latency.c

MYLIB_PATH := -L$(LUA_LIB) -L$(COMMON_LIB)
MYLIBS := -llua -lcommon

ifeq ($(CONFIG_RTE_BUILD_SHARED_LIB),y)
	MYLIBS += -lrte_pmd_ixgbe
	RPATHS += -rpath=$(LUA_LIB) -rpath=$(COMMON_LIB)
endif

# GUI support is a work in progress
ifeq ($(GUI),true)
	CFLAGS += -DGUI

	GTK_LIB = -L/usr/lib64/gtk-2.0/ \
		$(shell pkg-config --libs glib-2.0) \
		$(shell pkg-config --libs gtk+-2.0)

	MYLIB_PATH += -L$(GUI_LIB)
	MYLIBS += -lgui

	ifeq ($(CONFIG_RTE_BUILD_SHARED_LIB),y)
		RPATHS += -rpath=$(GUI_LIB)
	endif

	DEPDIRS-y += ../gui
endif

LDFLAGS += -g -lpcap $(RPATHS) $(MYLIB_PATH) $(MYLIBS) $(GTK_LIB) -lm

DEPDIRS-y += ../lib/common ../lib/lua

include $(RTE_SDK)/mk/rte.extapp.mk
