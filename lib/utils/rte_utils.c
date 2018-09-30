/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
 */

#include "rte_strings.h"
#include "rte_portlist.h"
#include "rte_utils.h"

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

#ifndef _RTE_STRING_FNS_H_
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

static inline void
set_portlist_bits(size_t low, size_t high, uint64_t *map)
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

		set_portlist_bits(ps, pe, &map);
	}

	if (portlist)
		*portlist = map;

	return 0;
}

/* isblank() needs _XOPEN_SOURCE >= 600 || _ISOC99_SOURCE, so use our
 * own. */
static int
isblank2(char c)
{
	if (c == ' ' || c == '\t' )
		return 1;
	return 0;
}

static int
isendofline(char c)
{
	if (c == '\n' || c == '\r' )
		return 1;
	return 0;
}

static int
iscomment(char c)
{
	if (c == '#')
		return 1;
	return 0;
}

static int
rte_isendoftoken(char c)
{
	if (!c || iscomment(c) || isblank2(c) || isendofline(c))
		return 1;
	return 0;
}

int
rte_atoip(const char *buf, int flags, void *res, unsigned ressize)
{
	unsigned int token_len = 0;
	char ip_str[INET6_ADDRSTRLEN+4+1]; /* '+4' is for prefixlen (if any) */
	struct rte_ipaddr ipaddr;
	char *prefix, *prefix_end;
	long prefixlen = 0;

	if (res && ressize < sizeof(struct rte_ipaddr))
		return -1;

	if (!buf || !*buf)
		return -1;

	while (!rte_isendoftoken(buf[token_len]))
		token_len++;

	/* if token is too big... */
	if (token_len >= INET6_ADDRSTRLEN+4)
		return -1;

	snprintf(ip_str, token_len+1, "%s", buf);

	/* convert the network prefix */
	if (flags & RTE_IPADDR_NETWORK) {
		prefix = strrchr(ip_str, '/');
		if (prefix == NULL)
			return -1;
		*prefix = '\0';
		prefix++;
		errno = 0;
		prefixlen = strtol(prefix, &prefix_end, 10);
		if (errno || (*prefix_end != '\0')
		    || prefixlen < 0 || prefixlen > RTE_PREFIXMAX)
			return -1;
		ipaddr.prefixlen = prefixlen;
	} else
		ipaddr.prefixlen = 0;

	/* convert the IP addr */
	if ((flags & RTE_IPADDR_V4) &&
	    inet_pton(AF_INET, ip_str, &ipaddr.ipv4) == 1 &&
	    prefixlen <= RTE_V4PREFIXMAX) {
		ipaddr.family = AF_INET;
		if (res)
			memcpy(res, &ipaddr, sizeof(ipaddr));
		return token_len;
	}

	if ((flags & RTE_IPADDR_V6) &&
	    inet_pton(AF_INET6, ip_str, &ipaddr.ipv6) == 1) {
		ipaddr.family = AF_INET6;
		if (res)
			memcpy(res, &ipaddr, sizeof(ipaddr));
		return token_len;
	}
	return -1;

}
