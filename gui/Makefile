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
# Created 2016 by Abhinandan Gujjar S (abhinandan.gujjar@ intel.com)


include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libgui.a

LUA_DIR = lua

GTK_INCLUDE := $(shell pkg-config --cflags glib-2.0)
GTK_INCLUDE += $(shell pkg-config --cflags gtk+-2.0)

#$(info $(GTK_INCLUDE))

CFLAGS += -O3 -g $(WERROR_FLAGS) -I$(RTE_SRCDIR)
CFLAGS += -I$(RTE_SRCDIR) -I$(RTE_SRCDIR)/../lib/common -I$(RTE_SRCDIR)/../app
CFLAGS += -I$(RTE_SRCDIR)/../lib/$(LUA_DIR)/src
CFLAGS += $(GTK_INCLUDE)
CFLAGS += -Wno-strict-prototypes -Wno-cast-qual

# all source are stored in SRCS-y
SRCS-y := pktgen-gui.c

SYMLINK-y-include := pktgen-gui.h

DEPDIRS-y += ../lib/common ../lib/$(LUA_DIR)

include $(RTE_SDK)/mk/rte.extlib.mk


