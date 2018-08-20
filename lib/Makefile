# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2018 Intel Corporation.

include $(RTE_SDK)/mk/rte.vars.mk

DIRS-y += common
DEPDIRS-common =

DIRS-y += utils
DEPDIRS-utils = common

ifeq ($(CONFIG_RTE_LIBRTE_VEC),)
DIRS-y += vec
DEPDIRS-vec += common utils
else
ifeq ($(CONFIG_RTE_LIBRTE_VEC),n)
DIRS-y += vec
DEPDIRS-vec += common utils
endif
endif

DIRS-y += lua
DEPDIRS-lua += vec utils

DIRS-y += cli
DEPDIRS-cli += lua utils

include $(RTE_SDK)/mk/rte.extsubdir.mk
