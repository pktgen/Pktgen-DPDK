/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2024> Intel Corporation.
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <sys/queue.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>

#include "lua_config.h"
#include "lua_stdio.h"
#include "lua_utils.h"

char *
lua_strtrim(char *str)
{
    if (!str || !*str)
        return str;

    /* trim white space characters at the front */
    while (isspace(*str))
        str++;

    /* Make sure the string is not empty */
    if (*str) {
        char *p = &str[strlen(str) - 1];

        /* trim trailing white space characters */
        while ((p >= str) && isspace(*p))
            p--;

        p[1] = '\0';
    }
    return *str ? str : NULL;
}
