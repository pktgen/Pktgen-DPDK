/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/**
 * @file
 *
 * String-related utility functions
 */

#ifndef _RTE_STRINGS_H_
#define _RTE_STRINGS_H_

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <rte_compat.h>
#include <rte_ether.h>
#include <rte_string_fns.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __numbits(_v)       __builtin_popcount(_v)

enum {
	STR_MAX_ARGVS = 64,             /**< Max number of args to support */
	STR_TOKEN_SIZE = 128,
};

/**
 * Trim a set of characters like "[]" or "{}" from the start and end of string.
 *
 * @param str
 *   A null terminated string to be trimmed.
 * @param set
 *   The <set> string is a set of two character values to be removed from the
 *   <str>. Removes only one set at a time, if you have more then one set to
 *   remove then you must call the routine for each set. The <set> string must
 *   be two characters and can be any characters you
 *   want to call a set.
 * @return
 *   Pointer to the trimmed string or NULL on error
 */
char * __rte_experimental rte_strtrimset(char *str, const char *set);

/**
 * Remove leading and trailing white space from a string.
 *
 * @param str
 *   String to be trimmed, must be null terminated
 * @return
 *   pointer to the trimmed string or NULL if <str> is Null or
 *   if string is empty then return pointer to <str>
 */
char * __rte_experimental rte_strtrim(char *str);

/**
 * Parse a string into a argc/argv list using a set of delimiters, but does
 * not handle quoted strings within the string being parsed.
 *
 * @param str
 *   String to be tokenized and will be modified, must be null terminated
 * @param delim
 *   A null terminated list of delimitors
 * @param entries
 *   A pointer to an array to place the token pointers
 * @param max_entries
 *   Max number of tokens to be placed in <entries>
 * @return
 *   The number of tokens in the <entries> array.
 */
int __rte_experimental rte_strtok(char *str, const char *delim, char **entries, int maxtokens);

/**
 * Parse a string into a argc/argv list using a set of delimiters, but does
 * handle quoted strings within the string being parsed
 *
 * @param str
 *   String to be tokenized and will be modified, null terminated
 * @param delim
 *   A null terminated list of delimitors
 * @param entries
 *   A pointer to an array to place the token pointers
 * @param max_entries
 *   Max number of tokens to be placed in <entries>
 * @return
 *   The number of tokens in the <entries> array.
 */
int __rte_experimental rte_strqtok(char *str, const char *delim, char **entries, int maxtokens);

/**
 * Parse a string <list> looking for <str> using delim character.
 *
 * @param list
 *   A string list of options with delim character between them.
 * @param str
 *   String to search for in <list>
 * @param delim
 *   A character string to use as a delim values
 * @return
 *   The index in the list of option strings, -1 if not found
 */
int __rte_experimental rte_stropt(const char *list, char *str, const char *delim);

/**
 * Parse a corelist string and return a list of the lcores.
 *
 * @param corelist
 *    The string defining a set of cores e.g. "1-8,22,25-29"
 * @param lcores
 *    The pointer to a uint16_t array to update with the core id.
 * @param len
 *    The length of the uint16_t array.
 * @return
 *    -1 error or number of cores in the string.
 */
int __rte_experimental rte_parse_corelist(const char *corelist,
	uint8_t *lcores, int len);

/**
 * Helper routine to compare two strings exactly
 *
 * @param s1
 *   Pointer to first string.
 * @param s2
 *   Pointer to second string.
 * @return
 *   0 failed to compare and 1 strings are equal.
 */
static inline int
rte_strmatch(const char * s1, const char * s2)
{
	if (!s1 || !s2)
		return 0;

	while((*s1 != '\0') && (*s2 != '\0')) {
		if (*s1++ != *s2++)
			return 0;
	}
	if (*s1 != *s2)
		return 0;

	return 1;
}

/**
 * Count the number of <c> characters in a string <s>
 *
 * @param s
 *   Null terminated string to search
 * @param c
 *   character to count
 * @return
 *   Number of times the character is in string.
 */
static inline int
rte_strcnt(char *s, char c)
{
	return (s == NULL || *s == '\0')
	       ? 0
	       : rte_strcnt(s + 1, c) + (*s == c);
}

/**
 * Convert a string Ethernet MAC address to the binary form
 *
 * @param a
 *   String containing the MAC address in two forms
 *      XX:XX:XX:XX:XX:XX or XXXX:XXXX:XXX
 * @param e
 *   pointer to a struct ether_addr to place the return value. If the value
 *   is null then use a static location instead.
 * @return
 *   Pointer to the struct ether_addr structure;
 */
static inline struct ether_addr *
rte_ether_aton(const char *a, struct ether_addr *e)
{
	int i;
	char *end;
	unsigned long o[ETHER_ADDR_LEN];
	static struct ether_addr ether_addr;

	if (!e)
		e = &ether_addr;

	i = 0;
	do {
		errno = 0;
		o[i] = strtoul(a, &end, 16);
		if (errno != 0 || end == a || (end[0] != ':' && end[0] != 0))
			return NULL;
		a = end + 1;
	} while (++i != sizeof (o) / sizeof (o[0]) && end[0] != 0);

	/* Junk at the end of line */
	if (end[0] != 0)
		return NULL;

	/* Support the format XX:XX:XX:XX:XX:XX */
	if (i == ETHER_ADDR_LEN) {
		while (i-- != 0) {
			if (o[i] > UINT8_MAX)
				return NULL;
			e->addr_bytes[i] = (uint8_t)o[i];
		}
		/* Support the format XXXX:XXXX:XXXX */
	} else if (i == ETHER_ADDR_LEN / 2) {
		while (i-- != 0) {
			if (o[i] > UINT16_MAX)
				return NULL;
			e->addr_bytes[i * 2] = (uint8_t)(o[i] >> 8);
			e->addr_bytes[i * 2 + 1] = (uint8_t)(o[i] & 0xff);
		}
		/* unknown format */
	} else
		return NULL;

	return e;
}

#ifndef _MTOA_
#define _MTOA_
/* char * inet_mtoa(char * buff, int len, struct ether_addr * eaddr) - Convert MAC address to ascii */
static __inline__ char *
inet_mtoa(char *buff, int len, struct ether_addr *eaddr) {
        snprintf(buff, len, "%02x:%02x:%02x:%02x:%02x:%02x",
                 eaddr->addr_bytes[0], eaddr->addr_bytes[1],
                 eaddr->addr_bytes[2], eaddr->addr_bytes[3],
                 eaddr->addr_bytes[4], eaddr->addr_bytes[5]);
        return buff;
}
#endif

#ifndef _MASK_SIZE_
#define _MASK_SIZE_
/* mask_size(uint32_t mask) - return the number of bits in mask */
static __inline__ int
mask_size(uint32_t mask) {
        if (mask == 0)
                return 0;
        else if (mask == 0xFF000000)
                return 8;
        else if (mask == 0xFFFF0000)
                return 16;
        else if (mask == 0xFFFFFF00)
                return 24;
        else if (mask == 0xFFFFFFFF)
                return 32;
        else {
                int i;
                for (i = 0; i < 32; i++)
                        if ( (mask & (1 << (31 - i))) == 0)
                                break;
                return i;
        }
}
#endif

#ifndef _NTOP4_
#define _NTOP4_
/* char * inet_ntop4(char * buff, int len, unsigned long ip_addr, unsigned long mask) - Convert IPv4 address to ascii */
static __inline__ char *
inet_ntop4(char *buff, int len, unsigned long ip_addr, unsigned long mask) {
        char lbuf[64];

        inet_ntop(AF_INET, &ip_addr, buff, len);
        if (mask != 0xFFFFFFFF) {
                snprintf(lbuf, sizeof(lbuf), "%s/%d", buff, mask_size(mask));
                rte_strlcpy(buff, lbuf, len);
        }
        return buff;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _RTE_STRINGS_H_ */
