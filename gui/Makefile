#
# Copyright (c) <2016>, Intel Corporation. All rights reserved.
#
# SPDX-License-Indentifier: BSD-3-Clause
#
# Created 2016 by Abhinandan Gujjar S (abhinandan.gujjar@intel.com)


include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libgui.a
LIBABIVER := 1

EXPORT_MAP := gui_version.map
LIBABIVER := 1

LUA_DIR = lua

GTK_INCLUDE := $(shell pkg-config --cflags glib-2.0) \
	       $(shell pkg-config --cflags gtk+-2.0)


#$(info $(GTK_INCLUDE))

CFLAGS += -O3 -g $(WERROR_FLAGS) -I$(RTE_SRCDIR) -D_GNU_SOURCE
CFLAGS += -I$(RTE_SRCDIR) -I$(RTE_SRCDIR)/../lib/common -I$(RTE_SRCDIR)/../app
CFLAGS += -I$(RTE_SRCDIR)/../lib/$(LUA_DIR)/src
CFLAGS += $(GTK_INCLUDE)
CFLAGS += -Wno-strict-prototypes -Wno-cast-qual

# all source are stored in SRCS-y
SRCS-y := pktgen-gui.c pktgen-gui-streams.c pktgen-gui-misc.c

SYMLINK-y-include := pktgen-gui.h pktgen-gui-streams.h pktgen-gui-cfg.h

DEPDIRS-y += ../lib/common ../lib/$(LUA_DIR)

include $(RTE_SDK)/mk/rte.extlib.mk


