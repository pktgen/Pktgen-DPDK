# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2019 Intel Corporation.

include $(RTE_SDK)/mk/rte.vars.mk

DIRS-y += common
DEPDIRS-common =

DIRS-y += plugin
DEPDIRS-plugin = common

ifeq ($(CONFIG_RTE_LIBRTE_UTILS),y)
else
DIRS-y += utils
DEPDIRS-utils = common
endif

ifeq ($(CONFIG_RTE_LIBRTE_VEC),y)
else
DIRS-y += vec
DEPDIRS-vec += common utils
endif

ifeq ($(CONFIG_RTE_LIBRTE_CLI),y)
else
DIRS-y += cli
DEPDIRS-cli += utils
endif

ifeq ($(CONFIG_RTE_LIBRTE_LUA),y)
else
DIRS-y += lua
DEPDIRS-lua += vec utils
DEPDIRS-cli += lua
endif

include $(RTE_SDK)/mk/rte.extsubdir.mk
