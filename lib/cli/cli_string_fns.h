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

#ifndef _CLI_STRING_FNS_H_
#define _CLI_STRING_FNS_H_

#include <stdio.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	STR_MAX_ARGVS = 64,             /**< Max number of args to support */
	STR_TOKEN_SIZE = 128,
};

typedef uint64_t	portlist_t;

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
int
rte_strsplit(char *string, int stringlen,
             char **tokens, int maxtokens, char delim);

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
char *rte_strtrimset(char *str, const char *set);

/**
 * Remove leading and trailing white space from a string.
 *
 * @param str
 *   String to be trimmed, must be null terminated
 * @return
 *   pointer to the trimmed string or NULL if <str> is Null or
 *   if string is empty then return pointer to <str>
 */
char *rte_strtrim(char *str);

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
int rte_strtok(char *str, const char *delim, char **entries, int maxtokens);

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
int rte_strqtok(char *str, const char *delim, char **entries, int maxtokens);

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
int rte_stropt(const char *list, char *str, const char *delim);

#ifndef _STRINGS_FNS_H_
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
#endif

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
int rte_parse_portlist(const char *str, portlist_t *portlist);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_STRING_FNS_H */
