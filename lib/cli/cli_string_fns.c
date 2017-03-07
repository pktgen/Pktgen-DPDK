/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
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

#include <rte_ethdev.h>

#include "cli.h"
#include "cli_search.h"
#include "cli_string_fns.h"

char *
rte_strtrimset(char *str, const char *set)
{
	char *p;
	size_t size;

	if (!str || !set || (strlen(set) != 2))
		return NULL;

	if ((size = strlen(str)) == 0)
		return str;

	/* Find the beginning set character, while trimming white space */
	while ((*str == set[0]) || isspace(*str))
		str++;

	if ((size = strlen(str)) == 0)
		return str;

	/* find and trim the closing character */
	p = &str[size - 1];
	while (p >= str && (isspace(*p) || (*p == set[1])))
		p--;

	p[1] = '\0';

	return str;
}

char *
rte_strtrim(char *str)
{
	char *p;
	size_t size;

	if (!str || (size = strlen(str)) == 0)
		return str;

	/* trim white space characters at the front */
	while (isspace(*str))
		str++;

	/* Make sure the string is not empty */
	if ((size = strlen(str)) == 0)
		return str;

	/* trim trailing white space characters, start at the end */
	p = &str[size - 1];
	while ((p >= str) && isspace(*p))
		p--;

	p[1] = '\0';

	return str;
}

int
rte_strtok(char *str, const char *delim, char **entries, int maxtokens)
{
	int i = 0;
	char *saved;

	if (!str || !delim || !entries || !maxtokens)
		return i;

	entries[i] = rte_strtrim(strtok_r(str, delim, &saved));
	if (entries[i] == NULL)
		return 0;

	for (i++; i < (maxtokens - 1); i++) {
		entries[i] = rte_strtrim(strtok_r(NULL, delim, &saved));

		if (!entries[i])/* Are we done yet */
			break;
	}
	entries[i] = NULL;

	return i;
}

int
rte_strqtok(char *str, const char *delim, char *argv[], int maxtokens)
{
	char *p, *start_of_word;
	int c;

	enum { INIT, WORD, STRING } state = INIT;
	int argc = 0;

	for (p = str; argc < (maxtokens - 1) && *p != '\0'; p++) {
		c = (unsigned char)*p;
		switch (state) {
		case INIT:
			if (strchr(delim, c) == NULL)
				continue;

			if ((c == '"') || (c == '\'')) {
				state = STRING;
				start_of_word = p + 1;
				continue;
			}
			state = WORD;
			start_of_word = p;
			continue;

		case STRING:
			if ((c == '"') || (c == '\'')) {
				*p = 0;
				argv[argc++] = start_of_word;
				state = INIT;
			}
			continue;

		case WORD:
			if (strchr(delim, c)) {
				*p = 0;
				argv[argc++] = start_of_word;
				state = INIT;
			}
			continue;
		}
	}

	if (state != INIT && argc < (maxtokens - 1))
		argv[argc++] = start_of_word;

	argv[argc] = NULL;

	return argc;
}

int
rte_split(char *str, int stringlen,
	  char **tokens, int maxtokens, char delim)
{
	char delims[2] = { delim, '\0' };

	if (stringlen == 0)
		return 0;

	str[stringlen] = '\0';	/* Force a null terminated string */
	return rte_strtok(str, delims, tokens, maxtokens);
}

int
rte_stropt(const char *list, char *str, const char *delim)
{
	int i = 0, n;
	char *buf;
	char *argv[CLI_MAX_ARGVS + 1];

	if ((list[0] == '%') && (list[1] == '#'))
		list += 2;

	buf = alloca(strlen(list) + 1);
	if (!buf)
		return -1;

	strcpy(buf, list);

	n = rte_strtok(buf, delim, argv, CLI_MAX_ARGVS);
	for (i = 0; i < n; i++)
		if (is_match(argv[i], str))
			return i;

	return -1;
}

uint32_t
rte_parse_list(char *str, const char *item_name, uint32_t max_items,
	       uint32_t *parsed_items, int check_unique_values)
{
	uint32_t nb_item, value, i, j;
	int value_ok;
	char c;

	/*
	 * First parse all items in the list and store their value.
	 */
	value = 0;
	nb_item = 0;
	value_ok = 0;
	for (i = 0; i < strnlen(str, STR_TOKEN_SIZE); i++) {
		c = str[i];
		if ((c >= '0') && (c <= '9')) {
			value = (unsigned int)(value * 10 + (c - '0'));
			value_ok = 1;
			continue;
		}
		if (c != ',') {
			printf("character %c is not a decimal digit\n", c);
			return 0;
		}
		if (!value_ok) {
			printf("No valid value before comma\n");
			return 0;
		}
		if (nb_item < max_items) {
			parsed_items[nb_item] = value;
			value_ok = 0;
			value = 0;
		}
		nb_item++;
	}
	if (nb_item >= max_items) {
		printf("Number of %s = %u > %u (maximum items)\n",
		       item_name, nb_item + 1, max_items);
		return 0;
	}
	parsed_items[nb_item++] = value;
	if (!check_unique_values)
		return nb_item;

	/*
	 * Then, check that all values in the list are differents.
	 * No optimization here...
	 */
	for (i = 0; i < nb_item; i++) {
		for (j = i + 1; j < nb_item; j++)
			if (parsed_items[j] == parsed_items[i]) {
				printf("duplicated %s %u at index %u and %u\n",
				       item_name, parsed_items[i], i, j);
				return 0;
			}
	}
	return nb_item;
}

static inline void
parse_set_list(uint32_t *map, size_t low, size_t high)
{
	do
		*map |= (1 << low++);
	while (low <= high);
}

int
rte_parse_portlist(const char *str, portlist_t *portlist)
{
	size_t ps, pe;
	const char *first, *last;
	char *end;
	uint32_t map = 0;

	if (!strcmp(str, "all")) {
		if (portlist) {
			int i;
			for (i = 0; i < rte_eth_dev_count(); i++)
				*portlist |= (1 << i);
		}
		return 0;
	}
	for (first = str, last = first;
	     first != NULL && last != NULL;
	     first = last + 1) {
		last = strchr(first, ',');

		errno = 0;
		ps = strtoul(first, &end, 10);
		if (errno != 0 || end == first ||
		    (end[0] != '-' && end[0] != 0 && end != last))
			return -1;

		/* Support for N-M portlist format */
		if (end[0] == '-') {
			errno = 0;
			first = end + 1;
			pe = strtoul(first, &end, 10);
			if (errno != 0 || end == first ||
			    (end[0] != 0 && end != last))
				return -1;
		} else
			pe = ps;

		if (ps > pe || pe >= sizeof(map) * 8)
			return -1;

		parse_set_list(&map, ps, pe);
	}

	if (portlist)
		*portlist = map;
	return 0;
}
