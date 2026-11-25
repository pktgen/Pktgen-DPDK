/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2026>, Intel Corporation.
 */

/**
 * @file
 * CLI argument map matching.
 *
 * Provides the cli_map structure and pattern-matching logic used to dispatch
 * CLI commands. Each map entry pairs a format string (with % tokens such as
 * %d, %4, %m, %P, %|opt1|opt2) with an integer index returned when the
 * user-supplied argc/argv matches that pattern.
 */

#ifndef _CLI_MAP_H_
#define _CLI_MAP_H_

#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

/** One entry in a command dispatch table. */
struct cli_map {
    int index;       /**< Value returned by cli_mapping() on a successful match */
    const char *fmt; /**< Space-separated format string; see cli_mapping() for tokens */
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
 *   %c - comma separated list string
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
struct cli_map *cli_mapping(struct cli_map *maps, int argc, char **argv);

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
 * Search a choice token in a cli_map format string for a matching item.
 *
 * @param fmt
 *   cli_map format string containing one or more %|opt1|opt2|… tokens.
 * @param item
 *   The string to look up within the choice token.
 * @param index
 *   Zero-based index of the choice token within @p fmt to search.
 * @return
 *   Zero-based position of @p item within the choice list, or -1 if not found.
 */
int cli_map_list_search(const char *fmt, char *item, int index);

#ifdef __cplusplus
}
#endif

#endif /* CLI_MAP_H */
