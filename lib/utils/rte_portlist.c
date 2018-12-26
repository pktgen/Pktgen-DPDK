/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

#include "rte_strings.h"
#include "rte_portlist.h"

#define SIZE_OF_PORTLIST      (sizeof(portlist_t) * 8)

int
rte_parse_portmask(const char *str, portlist_t *portmask)
{
	char *end = NULL;
	portlist_t pm;

	/* parse hexadecimal string */
	pm = strtoull(str, &end, 16);
	if ((str[0] == '\0') || (end == NULL) || (*end != '\0'))
		return -1;

	if (pm == 0)
		return -1;

	if (portmask)
		*portmask = pm;

	return 0;
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

char *
rte_portlist_string(uint64_t portlist, char *buf, int len)
{
	int i, k, j, cnt = 0;

	k = 0;
	for(i = 0; i < RTE_MAX_ETHPORTS; i++)
		if (portlist & (1UL << i))
			cnt++;

	memset(buf, 0, len);

	k = 0;
	j = 0;
	*buf = '\0';
	for(i = 0; (i < RTE_MAX_ETHPORTS) && (k < len); i++)
		if (portlist & (1UL << i)) {
			k += snprintf(&buf[k], len - k, "%d%s", i,
				(j >= cnt)? "" : ", ");
			j++;
		}

	if (k >= 2)
		buf[k - 2] = '\0';
	return buf;
}
char *
rte_print_portlist(FILE *f, uint64_t portlist, char *buf, int len)
{
	int i, k;

	if (!f)
		f = stdout;

	k = 0;
	*buf = '\0';
	for(i = 0; (i < RTE_MAX_ETHPORTS) && (k < len); i++)
		if (portlist & (1UL << i))
			k += snprintf(&buf[k], len - k, "%d, ", i);

	if (k >= 2)
		buf[k - 2] = '\0';
	return buf;
}
