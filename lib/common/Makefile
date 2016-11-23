#   BSD LICENSE
#
#   Copyright(c) 2010-2012 Intel Corporation. All rights reserved.
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     * Neither the name of Intel Corporation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libcommon.a
LIBABIVER := 1

CFLAGS += -O3 -g $(WERROR_FLAGS) -I$(RTE_SRCDIR)

EXPORT_MAP := common_version.map
LIBABIVER := 1

CFLAGS_wr_scrn.o := -Wno-format-nonliteral

# all source are stored in SRCS-y
SRCS-y := copyright_info.c port_config.c core_info.c cmdline_parse_args.c \
	lscpu.c utils.c coremap.c _pcap.c cksum.c l2p.c scrn.c

SYMLINK-y-include := cksum.h copyright_info.h core_info.h coremap.h \
	cmdline_parse_args.h cycles.h inet.h l2p.h lscpu.h mbuf.h _pcap.h \
	port_config.h scrn.h utils.h

include $(RTE_SDK)/mk/rte.extlib.mk
