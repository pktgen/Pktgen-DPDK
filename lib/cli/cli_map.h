/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
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
