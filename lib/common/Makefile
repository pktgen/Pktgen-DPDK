#   BSD LICENSE
#
#   Copyright(c) 2010-2012 Intel Corporation. All rights reserved.
#
#  SPDX-License-Identifier: BSD-3-Clause
#

include $(RTE_SDK)/$(RTE_TARGET)/.config
include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libcommon.a

CFLAGS += -O3 -g $(WERROR_FLAGS) -I$(RTE_SRCDIR) -D_GNU_SOURCE
CFLAGS += -DALLOW_EXPERIMENTAL_API

LDLIBS += -lrte_eal -lrte_mempool -lrte_kvargs -lrte_cmdline
 
EXPORT_MAP := common_version.map
LIBABIVER := 1

# all source are stored in SRCS-y
SRCS-y := copyright_info.c port_config.c core_info.c cmdline_parse_args.c \
	lscpu.c utils.c coremap.c _pcap.c cksum.c l2p.c

SYMLINK-y-include := cksum.h copyright_info.h core_info.h coremap.h \
	cmdline_parse_args.h pg_ether.h l2p.h lscpu.h mbuf.h _pcap.h \
	port_config.h utils.h

include $(RTE_SDK)/mk/rte.extlib.mk
