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

#include "wr_scrn.h"
#include "wr_copyright_info.h"

#define COPYRIGHT_MSG			"Copyright (c) <2010-2015>, Wind River Systems, Inc. All rights reserved."
#define POWERED_BY_DPDK			"Powered by Intel® DPDK"

static const char * intel_copyright[] = {
	"",
	"   BSD LICENSE",
	"",
	"   Copyright(c) 2010-2015 Intel Corporation. All rights reserved.",
	"   All rights reserved.",
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
static const char * wr_copyright[] = {
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

/**************************************************************************//**
*
* wr_print_copyright - Print out the copyright notices.
*
* DESCRIPTION
* Output the copyright notices.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
wr_print_copyright(const char * appname, const char * created_by)
{
	int		i;

	rte_printf_status("-----------------------\n");
	for(i=0; intel_copyright[i] != NULL; i++)
		rte_printf_status("  %s\n", intel_copyright[i]);
	rte_printf_status("-----------------------\n");

	rte_printf_status("    %s\n\n", COPYRIGHT_MSG);
	for(i=0; wr_copyright[i] != NULL; i++)
		rte_printf_status("  %s\n", wr_copyright[i]);

	wr_scrn_color(YELLOW, NO_CHANGE, OFF);
	rte_printf_status("  %s created by: %s -- >>> %s <<<\n", appname, created_by, POWERED_BY_DPDK);
	wr_scrn_color(BLUE, NO_CHANGE, OFF);
	rte_printf_status("-----------------------\n");
}

void
wr_logo(int row, int col, const char * appname)
{
	int		i;
	static const char * logo[] = {
		"#     #",
		"#  #  #     #    #    #  #####",
		"#  #  #     #    ##   #  #    #",
		"#  #  #     #    # #  #  #    #",
		"#  #  #     #    #  # #  #    #",
		"#  #  #     #    #   ##  #    #",
		" ## ##      #    #    #  #####",
		"",
		"######",
		"#     #     #    #    #  ######  #####",
		"#     #     #    #    #  #       #    #",
		"######      #    #    #  #####   #    #",
		"#   #       #    #    #  #       #####",
		"#    #      #     #  #   #       #   #",
		"#     #     #      ##    ######  #    #",
		"",
		" #####",
		"#     #   #   #   ####    #####  ######  #    #   ####",
		"#          # #   #          #    #       ##  ##  #",
		" #####      #     ####      #    #####   # ## #   ####",
		"      #     #         #     #    #       #    #       #",
		"#     #     #    #    #     #    #       #    #  #    #",
		" #####      #     ####      #    ######  #    #   ####",
		NULL
	};

	wr_scrn_cls();
	wr_scrn_color(GREEN, NO_CHANGE, BOLD);
	for(i=0, row++; logo[i] != NULL; i++)
		wr_scrn_printf(row++, 7, "%s", logo[i]);

	wr_scrn_color(MAGENTA, NO_CHANGE, OFF);
	wr_scrn_printf(++row, col, "%s", COPYRIGHT_MSG);
	wr_scrn_color(BLUE, NO_CHANGE, BOLD);
	wr_scrn_printf(++row, col+6, ">>> %s is %s <<<", appname, POWERED_BY_DPDK);
	wr_scrn_color(BLACK, NO_CHANGE, OFF);
	wr_scrn_pos(++row, 1);

	rte_delay_ms(1500);

    wr_scrn_cls();
    wr_scrn_pos(100, 1);
}

void
wr_splash_screen(int row, int col, const char * appname, const char * created_by)
{
	int		i;

	row = 3;
	wr_scrn_color(BLUE, NO_CHANGE, OFF);
	wr_scrn_printf(row++, col, "%s", COPYRIGHT_MSG);
	wr_scrn_color(GREEN, NO_CHANGE, BOLD);
	for(i=0, row++; wr_copyright[i] != NULL; i++)
		wr_scrn_printf(row++, 7, "%s", wr_copyright[i]);
	wr_scrn_color(BLUE, NO_CHANGE, BOLD);
	wr_scrn_printf(row++, col, "%s created by %s -- >>> %s <<<", appname, created_by, POWERED_BY_DPDK);
	wr_scrn_color(BLACK, NO_CHANGE, OFF);
	wr_scrn_pos(++row, 1);

	rte_delay_ms(1500);

    wr_scrn_cls();
    wr_scrn_pos(100, 1);
}

/**
 * Function returning string for Copyright message."
 * @return
 *     string
 */
const char *
wr_copyright_msg(void) {
	return COPYRIGHT_MSG;
}

/**
 * Function returning string for Copyright message."
 * @return
 *     string
 */
const char *
wr_powered_by(void) {
	return POWERED_BY_DPDK;
}

