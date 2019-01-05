/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

#include "copyright_info.h"

#define COPYRIGHT_MSG \
	"Copyright (c) <2010-2019>, Intel Corporation. All rights reserved."
#define COPYRIGHT_MSG_SHORT     "Copyright (c) <2010-2019>, Intel Corporation"
#define POWERED_BY_DPDK         "Powered by DPDK"

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
	printf("\n*** %s\n", COPYRIGHT_MSG);
	printf("*** %s created by: %s -- >>> %s <<<\n\n",
	       appname,
	       created_by,
	       POWERED_BY_DPDK);
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
