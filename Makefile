#   BSD LICENSE
#
#   Copyright(c) <2010-2019> Intel Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Created 2010-2018 by Keith Wiles @ intel.com

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

# GUI is a work in progress
ifeq ($(GUI),true)
DIRS-y += lib gui app
else
DIRS-y += lib app
endif

DEPDIRS-app += lib gui

export GUI

.PHONY: docs

include $(RTE_SDK)/mk/rte.extsubdir.mk

clean_objs = $(shell find . -name "*.a")
clean_objs += $(shell find . -name "x86_64*")
clean_objs += $(shell find . -name "*.d")
clean_objs += $(shell find . -name "*.o")
clean_objs += $(shell find . -name "*.cmd")

realclean:
	@if [ -n "$(clean_objs)" ]; then \
		rm -fr $(clean_objs); \
	fi
	rm -fr app/build

docs:
	@make -C docs html

pdf:
	@make -C docs latexpdf

cleandocs:
	@make -C docs clean
