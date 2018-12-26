/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

#include "rte_strings.h"

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
	int argc = 0;
	enum { INIT, WORD, STRING_QUOTE, STRING_TICK, STRING_BRACKET } state = WORD;

	if (!str || !delim || !argv || maxtokens == 0)
		return -1;

	/* Remove white space from start and end of string */
	s = rte_strtrim(str);

	start_of_word = s;
	for (p = s; (argc < maxtokens) && (*p != '\0'); p++) {
		int c = (unsigned char)*p;

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
			} else if (c == '{') {
				state = STRING_BRACKET;
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

		case STRING_BRACKET:
			if (c == '}') {
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

	n = strlen(list) + 2;

	buf = alloca(n);
	if (buf) {
		snprintf(buf, n, "%s", list);

		n = rte_strtok(buf, delim, argv, STR_MAX_ARGVS);

		for (i = 0; i < n; i++)
			if (rte_strmatch(argv[i], str))
				return i;
	}

	return -1;
}

int
rte_parse_corelist(const char *corelist, uint8_t *lcores, int len)
{
	int idx = 0;
	unsigned count = 0;
	char *end = NULL;
	int min, max, k;
	char cl_buf[128], *cl = cl_buf;

	if (corelist == NULL)
		return -1;

	memset(lcores, 0, len);

	strlcpy(cl, corelist, sizeof(cl_buf));

	/* Remove all blank characters ahead and after */
	cl = rte_strtrim(cl);

	/* Get list of cores */
	min = RTE_MAX_LCORE;
	k = 0;
	do {
		while (isblank(*cl))
			cl++;

		if (*cl == '\0')
			return -1;

		errno = 0;
		idx = strtoul(cl, &end, 10);
		if (errno || end == NULL)
			return -1;

		while (isblank(*end))
			end++;

		if (*end == '-')
			min = idx;
		else if ((*end == ',') || (*end == '\0')) {
			max = idx;

			if (min == RTE_MAX_LCORE)
				min = idx;

			for (idx = min; idx <= max; idx++) {
				lcores[idx] = 1;
				count++;
			}

			min = RTE_MAX_LCORE;
		} else
			return -1;

		cl = end + 1;

		if (k >= len)
			return -1;
	} while (*end != '\0');

	return count;
}
