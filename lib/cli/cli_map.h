/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2016-2017 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *
 * String-related functions as replacement for libc equivalents
 */

#ifndef _CLI_MAP_H_
#define _CLI_MAP_H_

#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

struct cli_map {
    int index;
    const char *fmt;
};

/**
 * Parse a string <list> looking for matching mapping.
 *
 * @param maps
 *   Array string points with mapping formats using a special % formats
 *   %d - decimal number 32bit
 *   %D - decimal number 64bit
 *   %h - hexadecimal number 32 bit number
 *   %H - hexadecimal number 64 bit number
 *   %P - Portlist
 *   %C - Corelist
 *   %s - string
 *   %m - MAC address format
 *   %4 - IPv4 address
 *   %6 - IPv6 address
 *   %k - kvargs list of <key>=<value>[,...] strings
 *   %l - list format, if constains space then quote the string first
 *   %|<fixed-list> - Fixed list of valid strings
 * @param argc
 *   Number of arguments in <argv>
 * @param argv
 *   Array of command line string pointers
 * @return
 *   return pointer map entry or NULL if not found
 */
struct cli_map *cli_mapping(struct cli_map *maps,
							int argc, char **argv);

/**
 * Dump out the map entry
 *
 * @param maps
 *   The cli_map structure pointer
 * @return
 *   None
 */
void cli_map_show(struct cli_map *m);

/**
 * Show the map table entries
 *
 * @param maps
 *   The cli_map structure pointer
 * @return
 *   None
 */
void cli_maps_show(struct cli_map *maps, int argc, char **argv);

/**
 * Dump out the map table entry matching the argc/argv
 *
 * @param maps
 *   The cli_map structure pointer
 * @param argc
 *   Number of argumemts
 * @param argv
 *   List of argument strings
 * @return
 *   None
 */
void cli_map_dump(struct cli_map *maps, int argc, char **argv);

/**
 * Determine index value for the list item
 *
 * @param fmt
 *   cli_map format string
 * @param item
 *   pointer to the item to search for in the list.
 * @param
 *   index value for which list in format string to scan
 * @return
 *  -1 on error or index into list selections.
 */
int cli_map_list_search(const char *fmt, char *item, int index);

#ifdef __cplusplus
}
#endif

#endif /* CLI_MAP_H */
