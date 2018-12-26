/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

#include "rte_strings.h"
#include "rte_atoip.h"

/* isblank() needs _XOPEN_SOURCE >= 600 || _ISOC99_SOURCE, so use our own. */
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
inet_ipton4(const char *src, unsigned char *dst)
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
inet_ipton6(const char *src, unsigned char *dst)
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
		    inet_ipton4(curtok, tp) > 0) {
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
inet_ipton(int af, const char *src, void *dst)
{
	switch (af) {
	case AF_INET:
		return inet_ipton4(src, dst);
	case AF_INET6:
		return inet_ipton6(src, dst);
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
	    inet_ipton(AF_INET, ip_str, &ipaddr.ipv4) == 1 &&
	    prefixlen <= RTE_V4PREFIXMAX) {
		ipaddr.family = AF_INET;
		if (res)
			memcpy(res, &ipaddr, sizeof(ipaddr));
		return token_len;
	}

	if ((flags & RTE_IPADDR_V6) &&
	    inet_ipton(AF_INET6, ip_str, &ipaddr.ipv6) == 1) {
		ipaddr.family = AF_INET6;
		if (res)
			memcpy(res, &ipaddr, sizeof(ipaddr));
		return token_len;
	}
	return -1;

}
