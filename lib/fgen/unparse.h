/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2021-2024> Intel Corporation
 */

#ifndef __UNPARSE_H
#define __UNPARSE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct unparse_s {
    void *data;        /**< Frame data */
    uint16_t data_len; /**< Length of the data buffer */
    uint16_t data_off; /**< Current offset into data frame */
    char *buffer;      /**< Output buffers pointer */
    int buf_len;       /**< length of buffer data */
    int used;          /**< Amount of used data in buffer */
} unparse_t;

/**
 * Return the data pointer to the packet data.
 *
 * @param f
 *   The unparse_t structure pointer.
 */
#define unparse_data(f) (f)->data

/**
 * Return the packet data length.
 *
 * @param f
 *   The unparse_t structure pointer.
 */
#define unparse_len(f) (f)->data_len

/**
 * Return the packet data length.
 *
 * @param f
 *   The unparse_t structure pointer.
 */
#define unparse_offset(f) (f)->data_off

/**
 * Return the packet data pointer at the given offset.
 *
 * @param f
 *   The unparse_t structure pointer.
 * @param t
 *   The pointer type cast.
 * @param o
 *   The offset into the packet.
 */
#define unparse_mtod_offset(f, t, o) ((t)((char *)unparse_data(f) + (o)))

/**
 * Return the first byte address of the packet data.
 *
 * @param f
 *   The frame_t structure pointer.
 * @param t
 *   The pointer type cast.
 */
#define unparse_mtod(f, t) unparse_mtod_offset(f, t, 0)

/**
 * Unparse the packet data
 *
 * @param data
 *   The frame data pointer.
 * @param frame_text
 *   The location to place the frame text.
 * @return
 *   -1 on error or length of frame text.
 */
int fgen_unparse(void *data, char **frame_text);

#ifdef __cplusplus
}
#endif

#endif /* __UNPARSE_H */
