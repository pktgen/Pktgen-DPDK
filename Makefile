#   BSD LICENSE
#
#   Copyright(c) 2010-2017 Intel Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Created 2010-2017 by Keith Wiles @ intel.com

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

clean_archives = $(shell find . -name "*.a")

realclean:
	@if [ -n "$(clean_archives)" ]; then \
		rm $(clean_archives); \
	fi

docs:
	@make -C docs html

pdf:
	@make -C docs latexpdf

cleandocs:
	@make -C docs clean
