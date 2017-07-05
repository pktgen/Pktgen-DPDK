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

#define SIZE_OF_PORTLIST      (sizeof(portlist_t) * 8)

char *
rte_strtrimset(char *str, const char *set)
{

	if (!str || !*str || !set || (strlen(set) != 2))
		return NULL;

	/* Find the beginning set character, while trimming white space */
	while ((*str == set[0]) || isspace(*str))
		str++;

	if (*str) {
		char *p = &str[strlen(str) - 1];

		while ((p >= str) && (isspace(*p) || (*p == set[1])))
			p--;

		p[1] = '\0';
	}

	return str;
}

char *
rte_strtrim(char *str)
{
	if (!str || !*str)
		return str;

	/* trim white space characters at the front */
	while(isspace(*str))
		str++;

	/* Make sure the string is not empty */
	if (*str) {
		char *p = &str[strlen(str) - 1];

		/* trim trailing white space characters */
		while((p >= str) && isspace(*p))
			p--;

		p[1] = '\0';
	}
	return str;
}

int
rte_strtok(char *str, const char *delim, char **entries, int maxtokens)
{
	int i = 0;
	char *saved;

	if (!str || !delim || !entries || !maxtokens)
		return -1;

	do {
		entries[i] = rte_strtrim(strtok_r(str, delim, &saved));
		str = NULL;
	} while(entries[i] && (++i < maxtokens));

	return i;
}

int
rte_strqtok(char *str, const char *delim, char *argv[], int maxtokens)
{
	char *p, *start_of_word, *s;
	int c, argc = 0;
	enum { INIT, WORD, STRING_QUOTE, STRING_TICK } state = WORD;

	if (!str || !delim || !argv || maxtokens == 0)
		return -1;

	/* Remove white space from start and end of string */
	s = rte_strtrim(str);

	start_of_word = s;
	for (p = s; (argc < maxtokens) && (*p != '\0'); p++) {
		c = (unsigned char)*p;

		if (c == '\\') {
			start_of_word = ++p;
			continue;
		}

		switch (state) {
		case INIT:
			if (c == '"') {
				state = STRING_QUOTE;
				start_of_word = p + 1;
			} else if (c == '\'') {
				state = STRING_TICK;
				start_of_word = p + 1;
			} else if (!strchr(delim, c)) {
				state = WORD;
				start_of_word = p;
			}
			break;

		case STRING_QUOTE:
			if (c == '"') {
				*p = 0;
				argv[argc++] = start_of_word;
				state = INIT;
			}
			break;

		case STRING_TICK:
			if (c == '\'') {
				*p = 0;
				argv[argc++] = start_of_word;
				state = INIT;
			}
			break;

		case WORD:
			if (strchr(delim, c)) {
				*p = 0;
				argv[argc++] = start_of_word;
				state = INIT;
				start_of_word = p + 1;
			}
			break;

		default:
			break;
		}
	}

	if ((state != INIT) && (argc < maxtokens))
		argv[argc++] = start_of_word;

	if ((argc == 0) && (p != str))
		argv[argc++] = str;

	return argc;
}

#ifdef RTE_LIBRTE_CLI
int
rte_strsplit(char *string, int stringlen,
             char **tokens, int maxtokens, char delim)
{
	char *s, d[3];

	if (stringlen <= 0)
		return 0;

	s = alloca(stringlen + 1);
	if (s) {
		memcpy(s, string, stringlen);
		s[stringlen] = '\0';

		snprintf(d, sizeof(d), "%c", delim);

		return rte_strtok(s, d, tokens, maxtokens);
	}
	return -1;
}
#endif

int
rte_stropt(const char *list, char *str, const char *delim)
{
	char *argv[STR_MAX_ARGVS + 1], *buf;
	size_t n, i;

	if (!list || !str || !delim)
		return -1;

	if ((list[0] == '%') && (list[1] == '|'))
		list += 2;

	if (!*list)
		return -1;

	n = strlen(list) + 1;

	buf = alloca(n);
	if (buf) {
		snprintf(buf, n - 1, "%s", list);

		n = rte_strtok(buf, delim, argv, STR_MAX_ARGVS);

		for (i = 0; i < n; i++)
			if (rte_strmatch(argv[i], str))
				return i;
	}

	return -1;
}

static inline void
parse_set_list(size_t low, size_t high, uint64_t *map)
{
	do {
		*map |= (1LL << low++);
	} while (low <= high);
}

#define MAX_SPLIT	64
/* portlist = N,N,N-M,N, ... */
int
rte_parse_portlist(const char *str, portlist_t *portlist)
{
	size_t ps, pe, n, i;
	char *split[MAX_SPLIT], *s, *p;
	uint64_t map = 0;

	if (!str || !*str)
		return -1;

	if (!strcmp(str, "all")) {
		if (portlist) {
			uint32_t i;
			for (i = 0; i < SIZE_OF_PORTLIST; i++)
				*portlist |= (1LL << i);
		}
		return 0;
	}

	n = strlen(str);
	s = alloca(n + 1);
	if (!s)
		return -1;

	memcpy(s, str, n);
	s[n] = '\0';

	n = rte_strtok(s, ",", split, MAX_SPLIT);
	if (!n)
		return 0;

	for(i = 0; i < n; i++) {
		p = strchr(split[i], '-');

		if (!p) {
			ps = strtoul(split[i], NULL, 10);
			pe = ps;
		} else {
			*p++ = '\0';
			ps = strtoul(split[i], NULL, 10);
			pe = strtoul(p, NULL, 10);
		}

		if ((ps > pe) || (pe >= (sizeof(map) * 8)))
			return -1;

		parse_set_list(ps, pe, &map);
	}

	if (portlist)
		*portlist = map;

	return 0;
}
