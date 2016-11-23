/*-
 * Copyright (c) <2010-2014>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2013 by Keith Wiles @ intel.com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <rte_version.h>
#include <rte_config.h>
#include <rte_atomic.h>
#include <rte_cycles.h>

#include "scrn.h"
#include "copyright_info.h"

#define COPYRIGHT_MSG \
        "Copyright (c) <2010-2016>, Intel Corporation. All rights reserved."
#define COPYRIGHT_MSG_SHORT     "Copyright (c) <2010-2016>, Intel Corporation"
#define POWERED_BY_DPDK         "Powered by Intel® DPDK"

#ifdef ENABLE_COPYRIGHT_OUTPUT
static const char *intel_copyright[] = {
	"",
	"   BSD LICENSE",
	"",
	"   Copyright(c) 2010-2016 Intel Corporation. All rights reserved.",
	"",
	"   Redistribution and use in source and binary forms, with or without",
	"   modification, are permitted provided that the following conditions",
	"   are met:",
	"",
	"     * Redistributions of source code must retain the above copyright",
	"       notice, this list of conditions and the following disclaimer.",
	"     * Redistributions in binary form must reproduce the above copyright",
	"       notice, this list of conditions and the following disclaimer in",
	"       the documentation and/or other materials provided with the",
	"       distribution.",
	"     * Neither the name of Intel Corporation nor the names of its",
	"       contributors may be used to endorse or promote products derived",
	"       from this software without specific prior written permission.",
	"",
	"   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS",
	"   \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT",
	"   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR",
	"   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT",
	"   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,",
	"   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT",
	"   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,",
	"   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY",
	"   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT",
	"   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE",
	"   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.",
	"",
	NULL
};
static const char *_copyright[] = {
	""
	"   Redistribution and use in source and binary forms, with or without modification, are",
	"   permitted provided that the following conditions are met:",
	"",
	"     1) Redistributions of source code must retain the above copyright notice,",
	"        this list of conditions and the following disclaimer.",
	"",
	"     2) Redistributions in binary form must reproduce the above copyright notice,",
	"        this list of conditions and the following disclaimer in the documentation and/or",
	"        other materials provided with the distribution.",
	"",
	"     3) Neither the name of Wind River Systems nor the names of its contributors may be",
	"        used to endorse or promote products derived from this software without specific",
	"        prior written permission.",
	"",
	"     4) The screens displayed by the application must contain the copyright notice as defined",
	"        above and can not be removed without specific prior written permission.",
	"",
	"   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"",
	"   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE",
	"   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE",
	"   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE",
	"   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL",
	"   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR",
	"   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER",
	"   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,",
	"   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE",
	"   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.",
	"",
	NULL
};
#endif

/**************************************************************************//**
 *
 * pg_print_copyright - Print out the copyright notices.
 *
 * DESCRIPTION
 * Output the copyright notices.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
print_copyright(const char *appname, const char *created_by)
{
#ifdef COPYRIGHT_OUTPUT_ENABLED
	int i;

	rte_printf_status("-----------------------\n");
	for (i = 0; intel_copyright[i] != NULL; i++)
		rte_printf_status("  %s\n", intel_copyright[i]);
	rte_printf_status("-----------------------\n");

	rte_printf_status("    %s\n\n", COPYRIGHT_MSG);
	for (i = 0; pg_copyright[i] != NULL; i++)
		rte_printf_status("  %s\n", pg_copyright[i]);

	scrn_color(YELLOW, NO_CHANGE, OFF);
	rte_printf_status("  %s created by: %s -- >>> %s <<<\n",
	                  appname,
	                  created_by,
	                  POWERED_BY_DPDK);
	scrn_color(BLUE, NO_CHANGE, OFF);
	rte_printf_status("-----------------------\n");
#else
	printf("   %s\n", COPYRIGHT_MSG);
	printf("   %s created by: %s -- >>> %s <<<\n\n",
	       appname,
	       created_by,
	       POWERED_BY_DPDK);
#endif
}

void
splash_screen(int row, int col, const char *appname, const char *created_by)
{
#ifdef ENABLE_COPYRIGHT_OUTPUT
	int i;

	row = 3;
	scrn_color(BLUE, NO_CHANGE, OFF);
	scrn_printf(row++, col, "%s", COPYRIGHT_MSG);
	scrn_color(GREEN, NO_CHANGE, BOLD);
	for (i = 0, row++; pg_copyright[i] != NULL; i++)
		scrn_printf(row++, 7, "%s", pg_copyright[i]);
	scrn_color(BLUE, NO_CHANGE, BOLD);
	scrn_printf(row++,
	               col,
	               "%s created by %s -- >>> %s <<<",
	               appname,
	               created_by,
	               POWERED_BY_DPDK);
	scrn_color(BLACK, NO_CHANGE, OFF);
	scrn_pos(++row, 1);

	rte_delay_ms(1500);

	scrn_cls();
	scrn_pos(100, 1);
#else
	(void)row;
	(void)col;
	(void)appname;
	(void)created_by;
#endif
}

/**
 * Function returning string for Copyright message."
 * @return
 *     string
 */
const char *
copyright_msg(void) {
	return COPYRIGHT_MSG;
}

/**
 * Function returning short string for Copyright message."
 * @return
 *     string
 */
const char *
copyright_msg_short(void) {
	return COPYRIGHT_MSG_SHORT;
}

/**
 * Function returning string for Copyright message."
 * @return
 *     string
 */
const char *
powered_by(void) {
	return POWERED_BY_DPDK;
}
