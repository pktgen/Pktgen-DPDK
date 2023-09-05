/*-
 * Copyright(c) <2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2017 Intel Corporation
 *
 * Taken from: dpdk.org (https://dpdk.org/) and modified
 */

#ifndef _PKTGEN_TXBUFF_H_
#define _PKTGEN_TXBUFF_H_

#include <rte_mbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*tx_buffer_error_fn)(struct rte_mbuf **unsent, uint16_t count, void *userdata);

/**
 * Structure used to buffer packets for future Tx
 * Used by APIs pg_eth_tx_buffer and pg_eth_tx_buffer_flush
 */
struct eth_tx_buffer {
    uint16_t size;   /**< Size of buffer for buffered Tx */
    uint16_t length; /**< Number of packets in the array */
    uint16_t pid;    /**< Port ID */
    uint16_t qid;    /**< Queue ID */
    /** Pending packets to be sent on explicit flush or when full */
    struct rte_mbuf *pkts[];
};

/**
 * Calculate the size of the Tx buffer.
 *
 * @param sz
 *   Number of stored packets.
 */
#define TX_BUFFER_SIZE(sz) (sizeof(struct eth_tx_buffer) + (sz) * sizeof(struct rte_mbuf *))

/**
 * Send any packets queued up for transmission on a port and HW queue
 *
 * This causes an explicit flush of packets previously buffered via the
 * tx_buffer() function. It returns the number of packets successfully
 * sent to the NIC.
 *
 * @param buffer
 *   Buffer of packets to be transmit.
 * @return
 *   The number of packets successfully sent to the Ethernet device.
 */
static inline uint16_t
tx_buffer_flush(struct eth_tx_buffer *buffer)
{
    uint16_t sent, tot_sent = 0;
    uint16_t to_send = buffer->length;
    struct rte_mbuf **pkts = buffer->pkts;

    if (to_send == 0)
        return 0;
    buffer->length = 0;

    do {
        sent = rte_eth_tx_burst(buffer->pid, buffer->qid, pkts, to_send);
        to_send -= sent;
        pkts += sent;
        tot_sent += sent;
    } while(to_send > 0);

    return tot_sent;
}

/**
 * Buffer a single packet for future transmission on a port and queue
 *
 * This function takes a single mbuf/packet and buffers it for later
 * transmission on the particular port and queue specified. Once the buffer is
 * full of packets, an attempt will be made to transmit all the buffered
 * packets. In case of error, where not all packets can be transmitted, a
 * callback is called with the unsent packets as a parameter. If no callback
 * is explicitly set up, the unsent packets are just freed back to the owning
 * mempool. The function returns the number of packets actually sent i.e.
 * 0 if no buffer flush occurred, otherwise the number of packets successfully
 * flushed
 *
 * @param buffer
 *   Buffer used to collect packets to be sent.
 * @param tx_pkt
 *   Pointer to the packet mbuf to be sent.
 * @return
 *   0 = packet has been buffered for later transmission
 *   N > 0 = packet has been buffered, and the buffer was subsequently flushed,
 *     causing N packets to be sent, and the error callback to be called for
 *     the rest.
 */
static __rte_always_inline uint16_t
tx_buffer(struct eth_tx_buffer *buffer, struct rte_mbuf *tx_pkt)
{
    buffer->pkts[buffer->length++] = tx_pkt;
    if (buffer->length < buffer->size)
        return 0;

    return tx_buffer_flush(buffer);
}

/**
 * Buffer a vector of packets for future transmission on a port and queue
 *
 * This function takes a vector of mbufs/packets and buffers it for later
 * transmission on the particular port and queue specified. Once the buffer is
 * full of packets, an attempt will be made to transmit all the buffered
 * packets. In case of error, where not all packets can be transmitted, a
 * callback is called with the unsent packets as a parameter. If no callback
 * is explicitly set up, the unsent packets are just freed back to the owning
 * mempool. The function returns the number of packets actually sent i.e.
 * 0 if no buffer flush occurred, otherwise the number of packets successfully
 * flushed
 *
 * @param buffer
 *   Buffer used to collect packets to be sent.
 * @param tx_pkt
 *   Pointer to the vector of mbufs to be sent.
 * @param nb_pkts
 *   Number of packets to be sent in the vector.
 * @return
 *   0 = packet has been buffered for later transmission
 *   N > 0 = packet has been buffered, and the buffer was subsequently flushed,
 *     causing N packets to be sent, and the error callback to be called for
 *     the rest.
 */
static __rte_always_inline uint16_t
tx_buffer_bulk(struct eth_tx_buffer *buffer, struct rte_mbuf **tx_pkt, uint16_t nb_pkts)
{
    for (uint16_t i = 0; i < nb_pkts; i++) {
        tx_buffer(buffer, *tx_pkt);
        tx_pkt++;
    }

    return nb_pkts;
}

/**
 * Initialize default values for buffered transmitting
 *
 * @param buffer
 *   Tx buffer to be initialized.
 * @param size
 *   Buffer size
 * @param pid
 *   Port ID
 * @param qid
 *   Queue ID
 * @return
 *   0 if no error
 */
static __inline__ int
tx_buffer_init(struct eth_tx_buffer *buffer, uint16_t size, uint16_t pid, uint16_t qid)
{
    int ret = 0;

    if (buffer == NULL) {
        printf("Cannot initialize NULL buffer\n");
        return -EINVAL;
    }

    buffer->size = size;
    buffer->pid  = pid;
    buffer->qid  = qid;

    return ret;
}

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_TXBUFF_H_ */
