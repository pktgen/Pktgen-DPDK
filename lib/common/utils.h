/*-
 * Copyright(c) <2010-2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2014 by Keith Wiles @ intel.com */

#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The function is a wrapper around strdup() and will free the previous string
 * if the pointer is present.
 */

static __inline__ char *
pg_strdupf(char *str, char *new) {
	if (str) free(str);
	return (new == NULL) ? NULL : strdup(new);
}

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
static __inline__ char *
pg_strtrimset(char *str, const char *set)
{
	int len;

	len = strlen(set);
	if ( (len == 0) || (len & 1) )
		return NULL;

	for (; set && (set[0] != '\0'); set += 2) {
		if (*str != *set)
			continue;

		if (*str == *set++)
			str++;

		len = strlen(str);
		if (len && (str[len - 1] == *set) )
			str[len - 1] = '\0';
	}
	return str;
}

uint32_t pg_strparse(char *s,
			    const char *delim,
			    char **entries,
			    uint32_t max_entries);
#if 0
char *pg_strtrim(char *line);
#endif
char *pg_strccpy(char *t, char *f, const char *str);

#ifdef __cplusplus
}
#endif

#endif /* _UTILS_H_ */
