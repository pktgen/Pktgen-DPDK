#   BSD LICENSE
#
#   Copyright(c) 2010-2017 Intel Corporation.
#   All rights reserved.
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

realclean:
	@rm -fr app/app
	@rm -fr lib/common/lib
	@rm -fr lib/cli/lib
	@rm -fr lib/lua/src/lib
	@rm -fr app/$(RTE_TARGET)
	@rm -fr lib/common/$(RTE_TARGET)
	@rm -fr lib/cli/$(RTE_TARGET)
	@rm -fr lib/lua/src/$(RTE_TARGET)

docs:
	@make -C docs html

pdf:
	@make -C docs latexpdf

cleandocs:
	@make -C docs clean
