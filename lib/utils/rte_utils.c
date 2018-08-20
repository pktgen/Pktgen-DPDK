/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
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

/*
 * Like inet_aton() but without all the hexadecimal and shorthand.
 * return:
 *      1 if `src' is a valid dotted quad, else 0.
 * notice:
 *      does not touch `dst' unless it's returning 1.
 * author:
 *      Paul Vixie, 1996.
 */
static int
inet_pton4(const char *src, unsigned char *dst)
{
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	unsigned char tmp[RTE_INADDRSZ], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr(digits, ch)) != NULL) {
			unsigned int new = *tp * 10 + (pch - digits);

			if (new > 255)
				return 0;
			if (!saw_digit) {
				if (++octets > 4)
					return 0;
				saw_digit = 1;
			}
			*tp = (unsigned char)new;
		} else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return 0;
			*++tp = 0;
			saw_digit = 0;
		} else
			return 0;
	}
	if (octets < 4)
		return 0;

	memcpy(dst, tmp, RTE_INADDRSZ);
	return 1;
}

/*
 * Convert presentation level address to network order binary form.
 * return:
 *      1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *      (1) does not touch `dst' unless it's returning 1.
 *      (2) :: in a full address is silently ignored.
 * credit:
 *      inspired by Mark Andrews.
 * author:
 *      Paul Vixie, 1996.
 */
static int
inet_pton6(const char *src, unsigned char *dst)
{
	static const char xdigits_l[] = "0123456789abcdef",
	                                xdigits_u[] = "0123456789ABCDEF";
	unsigned char tmp[RTE_IN6ADDRSZ], *tp = 0, *endp = 0, *colonp = 0;
	const char *xdigits = 0, *curtok = 0;
	int ch = 0, saw_xdigit = 0, count_xdigit = 0;
	unsigned int val = 0;
	unsigned dbloct_count = 0;

	memset((tp = tmp), '\0', RTE_IN6ADDRSZ);
	endp = tp + RTE_IN6ADDRSZ;
	colonp = NULL;
	/* Leading :: requires some special handling. */
	if (*src == ':')
		if (*++src != ':')
			return 0;
	curtok = src;
	saw_xdigit = count_xdigit = 0;
	val = 0;

	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			if (count_xdigit >= 4)
				return 0;
			val <<= 4;
			val |= (pch - xdigits);
			if (val > 0xffff)
				return 0;
			saw_xdigit = 1;
			count_xdigit++;
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!saw_xdigit) {
				if (colonp)
					return 0;
				colonp = tp;
				continue;
			} else if (*src == '\0')
				return 0;
			if (tp + sizeof(int16_t) > endp)
				return 0;
			*tp++ = (unsigned char) ((val >> 8) & 0xff);
			*tp++ = (unsigned char) (val & 0xff);
			saw_xdigit = 0;
			count_xdigit = 0;
			val = 0;
			dbloct_count++;
			continue;
		}
		if (ch == '.' && ((tp + RTE_INADDRSZ) <= endp) &&
		    inet_pton4(curtok, tp) > 0) {
			tp += RTE_INADDRSZ;
			saw_xdigit = 0;
			dbloct_count += 2;
			break; /* '\0' was seen by inet_pton4(). */
		}
		return 0;
	}
	if (saw_xdigit) {
		if (tp + sizeof(int16_t) > endp)
			return 0;
		*tp++ = (unsigned char) ((val >> 8) & 0xff);
		*tp++ = (unsigned char) (val & 0xff);
		dbloct_count++;
	}
	if (colonp != NULL) {
		/* if we already have 8 double octets, having a colon means error */
		if (dbloct_count == 8)
			return 0;

		/*
		 * Since some memmove()'s erroneously fail to handle
		 * overlapping regions, we'll do the shift by hand.
		 */
		const int n = tp - colonp;
		int i;

		for (i = 1; i <= n; i++) {
			endp[-i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return 0;
	memcpy(dst, tmp, RTE_IN6ADDRSZ);
	return 1;
}

/*
 * Convert from presentation format (which usually means ASCII printable)
 *      to network format (which is usually some kind of binary format).
 * @return:
 *      1 if the address was valid for the specified address family
 *      0 if the address wasn't valid (`dst' is untouched in this case)
 *      -1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *      Paul Vixie, 1996.
 */
static int
inet_pton(int af, const char *src, void *dst)
{
	switch (af) {
	case AF_INET:
		return inet_pton4(src, dst);
	case AF_INET6:
		return inet_pton6(src, dst);
	default:
		errno = EAFNOSUPPORT;
		return -1;
	}
	/* NOTREACHED */
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
