/**
 * <COPYRIGHT_TAG>
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
/* Created 2014 by Keith Wiles @ intel.com */

#include <rte_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <termios.h>

#include "wr_utils.h"

#define MAX_PARSE_SIZE		256

/******************************************************************************
* wr_strtrim  - Remove leading and trailing white space from a string.
*
* SYNOPSIS
* char * wr_strtrim
*     (
*     char *    str
*     )
*
* DESCRIPTION
* Remove leading and trailing white space from a string.
*
* RETURNS: pointer to the trimmed string or NULL <str> is Null.
*
* ERRNO: N/A
*
* \NOMANUAL
*/
char *
wr_strtrim( char * str )
{
    char	  * p;
    int			len;

    if ( (str != NULL) && (len = strlen(str)) ) {
		/* skip white spaces at the front of the string */
		for (; *str != 0; str++ )
			if ( (*str != ' ') && (*str != '\t') && (*str != '\r') && (*str != '\n') )
				break;

		len = strlen(str);
		if ( len == 0 )
			return str;

		// Trim trailing characters
		for ( p = &str[len-1]; p > str; p-- ) {
			if ( (*p != ' ') && (*p != '\t') && (*p != '\r') && (*p != '\n') )
				break;
			*p = '\0';
		}
    }
    return str;
}

uint32_t
wr_strparse(char * str, const char * delim, char ** entries, uint32_t max_entries)
{
    uint32_t	i;
    char	  * saved;

    if ( (str == NULL) || (delim == NULL) || (entries == NULL) || (max_entries == 0) )
    	return 0;

    memset(entries, '\0', (sizeof(char *) * max_entries));

    for(i = 0; i < max_entries; i++) {
        entries[i] = strtok_r(str, delim, &saved);
        str = NULL;
        if ( entries[i] == NULL )		// We are done.
            break;

        entries[i] = wr_strtrim(entries[i]);
    }

    return i;
}

static uint32_t
skip_lst( char f, const char * lst)
{
	for( ; *lst != '\n'; lst++) {
		if ( f == *lst )
			return 1;
	}
	return 0;
}

char *
wr_strccpy(char * t, char * f, const char * lst)
{
	if ( (t == NULL) || (f == NULL) )
		return NULL;

	*t ='\0';
	if ( *f == '\0' )
		return t;

	while( *f != '\0' ) {
		if ( skip_lst(*f, lst) )
			f++;
		else
			*t++ = *f++;
	}
	*t = '\0';
	return t;
}
