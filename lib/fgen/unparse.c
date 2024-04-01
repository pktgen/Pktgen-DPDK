/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2021-2024> Intel Corporation
 */
/* Created using ideas from DPDK tgen */

#include <stdint.h>        // for uint32_t, uint16_t, int32_t, uint8_t
#include <stdbool.h>
#include <netinet/in.h>        // for ntohs, htonl, htons
#include <net/ethernet.h>

#include <rte_common.h>
#include <rte_log.h>
#include <pg_strings.h>
#include <pg_inet.h>
#include <utils.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_tcp.h>
#include <rte_vxlan.h>

#include "fgen.h"
#include "unparse.h"

static __attribute__((__format__(__printf__, 2, 0))) int
_append(unparse_t *up, const char *format, ...)
{
    va_list ap;
    char str[FGEN_MAX_STRING_LENGTH];
    int ret, nbytes;

    va_start(ap, format);
    ret = vsnprintf(str, sizeof(str), format, ap);
    va_end(ap);

    /* First time just allocate some memory to use for buffer */
    if (up->buffer == NULL) {
        up->buffer = calloc(1, 4 * FGEN_EXTRA_SPACE);
        if (up->buffer == NULL) {
            printf("Buffer is NULL\n");
            return -1;
        }
        up->buf_len = (4 * FGEN_EXTRA_SPACE);
        up->used    = 0;
    }

    nbytes = (ret + up->used) + FGEN_EXTRA_SPACE;

    /* Increase size of buffer if required */
    if (nbytes > up->buf_len) {

        /* Make sure the max length is capped to a max size */
        if (nbytes > FGEN_MAX_BUF_LEN)
            return -1;

        /* expand the buffer space */
        char *p = realloc(up->buffer, nbytes);

        if (p == NULL)
            return -1;

        up->buffer  = p;
        up->buf_len = nbytes;
    }

    /* Add the new string data to the buffer */
    up->used = strlcat(up->buffer, str, up->buf_len);

    return 0;
}

static int
_unparse_vlan(unparse_t *up, bool is_dot1ad)
{
    (void)up;
    (void)is_dot1ad;

    return 0;
}

static int
_unparse_dot1ad(unparse_t *up)
{
    _append(up, "Dot1Q(");
    if (_unparse_vlan(up, true) < 0)
        return -1;
    _append(up, ")/");
    return 0;
}

static int
_unparse_dot1q(unparse_t *up)
{
    _append(up, "Dot1Q(");
    if (_unparse_vlan(up, false) < 0)
        return -1;
    _append(up, ")/");

    return 0;
}

static int
_unparse_ipv4(unparse_t *up)
{
    struct rte_ipv4_hdr *ip;
    char buf[64];

    ip = unparse_mtod(up, struct rte_ipv4_hdr *);

    unparse_offset(up) += sizeof(struct rte_ipv4_hdr);

    _append(up, "IPv4(");

    inet_ntop4(buf, sizeof(buf), ip->dst_addr, 0xFFFFFFFF);
    _append(up, "dst=%s", buf);

    inet_ntop4(buf, sizeof(buf), ip->src_addr, 0xFFFFFFFF);
    _append(up, ",src=%s", buf);

    _append(up, ")/");

    return 0;
}

static int
_unparse_ether(unparse_t *up)
{
    struct ether_header *eth;
    struct ether_addr *addr;

    eth = unparse_mtod(up, struct ether_header *);

    _append(up, "Ether(");
    addr = (struct ether_addr *)&eth->ether_dhost;
    _append(up, "dst=%02X:%02X:%02X:%02X:%02X:%02X", addr->ether_addr_octet[0],
            addr->ether_addr_octet[1], addr->ether_addr_octet[2], addr->ether_addr_octet[3],
            addr->ether_addr_octet[4], addr->ether_addr_octet[5]);
    addr = (struct ether_addr *)&eth->ether_shost;
    _append(up, ",src=%02X:%02X:%02X:%02X:%02X:%02X", addr->ether_addr_octet[0],
            addr->ether_addr_octet[1], addr->ether_addr_octet[2], addr->ether_addr_octet[3],
            addr->ether_addr_octet[4], addr->ether_addr_octet[5]);
    _append(up, ")/");
    unparse_offset(up) += sizeof(struct ether_header);

    switch (ntohs(eth->ether_type)) {
    case RTE_ETHER_TYPE_VLAN:
        return _unparse_dot1ad(up);
    case RTE_ETHER_TYPE_QINQ:
        return _unparse_dot1q(up);
    case RTE_ETHER_TYPE_IPV4:
        return _unparse_ipv4(up);
    default:
        break;
    }
    return -1;
}

int
fgen_unparse(void *data, char **frame_text)
{
    unparse_t *up = NULL;
    int len       = -1;

    up = calloc(1, sizeof(*up));
    if (!up)
        goto leave;

    up->data = data;

    if (_unparse_ether(up) < 0)
        goto leave;

    *frame_text = up->buffer;
    len         = strlen(up->buffer);

leave:
    if (up) {
        free(up->buffer);
        free(up);
    }

    return len;
}
