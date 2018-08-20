/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
 */

/**
 * @file
 *
 * String-related utility functions
 */

#ifndef _RTE_UTILS_H_
#define _RTE_UTILS_H_

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#include <rte_compat.h>
#include <rte_string_fns.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN     6

/**
 * Ethernet address:
 * A universally administered address is uniquely assigned to a device by its
 * manufacturer. The first three octets (in transmission order) contain the
 * Organizationally Unique Identifier (OUI). The following three (MAC-48 and
 * EUI-48) octets are assigned by that organization with the only constraint
 * of uniqueness.
 * A locally administered address is assigned to a device by a network
 * administrator and does not contain OUIs.
 * See http://standards.ieee.org/regauth/groupmac/tutorial.html
 */
struct ether_addr {
	uint8_t addr_bytes[ETHER_ADDR_LEN]; /**< Addr bytes in tx order */
} __attribute__((__packed__));

#endif

#define RTE_IPADDR_V4      0x01
#define RTE_IPADDR_V6      0x02
#define RTE_IPADDR_NETWORK 0x04

#define RTE_INADDRSZ       4
#define RTE_IN6ADDRSZ      16
#define RTE_PREFIXMAX      128
#define RTE_V4PREFIXMAX    32

struct rte_ipaddr {
	uint8_t family;
	union {
		struct in_addr ipv4;
		struct in6_addr ipv6;
	};
	unsigned int prefixlen; /* in case of network only */
};

enum {
	STR_MAX_ARGVS = 64,             /**< Max number of args to support */
	STR_TOKEN_SIZE = 128,
};

#ifndef _RTE_STRING_FNS_H_
/**
 * Takes string <string> parameter and splits it at character <delim>
 * up to maxtokens-1 times - to give <maxtokens> resulting tokens. Like
 * strtok or strsep functions, this modifies its input string, by replacing
 * instances of <delim> with '\\0'. All resultant tokens are returned in the
 * <tokens> array which must have enough entries to hold <maxtokens>.
 *
 * @param string
 *   The input string to be split into tokens
 * @param stringlen
 *   The max length of the input buffer
 * @param tokens
 *   The array to hold the pointers to the tokens in the string
 * @param maxtokens
 *   The number of elements in the tokens array. At most, <maxtokens>-1 splits
 *   of the string will be done.
 * @param delim
 *   The character on which the split of the data will be done
 * @return
 *   The number of tokens in the array.
 */
int __rte_experimental
rte_strsplit(char *string, int stringlen,
             char **tokens, int maxtokens, char delim);
#endif

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
 * Parse a portlist string into a mask or bitmap value.
 *
 * @param str
 *   String to parse
 * @param portlist
 *   Pointer to uint64_t value for returned bitmap
 * @return
 *   -1 on error or 0 on success.
 */
int __rte_experimental rte_parse_portlist(const char *str, portlist_t *portlist);

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


/**
 * Convert an IPv4/v6 address into a binary value.
 *
 * @param buf
 *   Location of string to convert
 * @param flags
 *   Set of flags for converting IPv4/v6 addresses and netmask.
 * @param res
 *   Location to put the results
 * @param ressize
 *   Length of res in bytes.
 * @return
 *   0 on OK and -1 on error
 */
int __rte_experimental  rte_atoip(const char *buf, int flags, void *res, unsigned ressize);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_UTILS_H */
