#   BSD LICENSE
#
#   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
#
#  SPDX-License-Identifier: BSD-3-Clause
#

include $(RTE_SDK)/mk/rte.vars.mk

DIRS-y += common
DEPDIRS-y = common

ifeq ($(CONFIG_RTE_LIBRTE_LUA),)
DIRS-y += lua
DEPDIRS-y += lua
else
ifeq ($(CONFIG_RTE_LIBRTE_LUA),n)
DIRS-y += lua
DEPDIRS-y += lua
endif
endif

ifeq ($(CONFIG_RTE_LIBRTE_CLI),)
DIRS-y += cli
DEPDIRS-y += cli
else
ifeq ($(CONFIG_RTE_LIBRTE_CLI),n)
DIRS-y += cli
DEPDIRS-y += cli
endif
endif

include $(RTE_SDK)/mk/rte.extsubdir.mk
