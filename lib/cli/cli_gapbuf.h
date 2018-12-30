/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */
/* inspired by an email/code written by: Joseph H. Allen, 9/10/89 */

#include <stdlib.h>
#include <string.h>

#include <rte_debug.h>

#ifndef _CLI_GAPBUF_H_
#define _CLI_GAPBUF_H_

/**
 * @file
 * CLI Gap Buffer support
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#define GB_DEFAULT_GAP_SIZE     8

struct gapbuf {
	char *buf;          /** Pointer to start of buffer */
	char *ebuf;         /** Pointer to end of buffer */
	char *point;        /** Pointer to point in the buffer */
	char *gap;          /** Pointer to the start of the gap */
	char *egap;         /** Pointer to the end of the gap */
};

/**
 * Create the Gap Buffer structure
 *
 * @return
 *   NULL on error or pointer to struct gapbuf
 */
struct gapbuf *gb_create(void);

/**
 * Destroy
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   N/A
 */
void gb_destroy(struct gapbuf *gb);

/**
 * Allocate buffer and initialize, if buffer exist free and reallocate.
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param size
 *   Init the gap buffer to the size given
 * @return
 *   0 is OK or Error
 */
int gb_init_buf(struct gapbuf *gb, int size);

/**
 * Reset the gap buffer
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   0 is OK or Error
 */
int gb_reset_buf(struct gapbuf *gb);

/**
 * Copy the buffer data into a given buffer
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param dst
 *   Location to copy the data into
 * @param size
 *   Total number of bytes to copy
 * @return
 *   Number of bytes copied into the buffer
 */
uint32_t gb_copy_to_buf(struct gapbuf *gb, char *dst, uint32_t size);

/**
 * Print out a debug list of the Gap buffer and pointers
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param msg
 *   Message to print out befor dump of buffer data
 * @return
 *   N/A
 */
void gb_dump(struct gapbuf *gb, const char * msg);

/********************************************************/

/**
 * Return the number of bytes total in the buffer includes gap size
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   The number of bytes in the buffer
 */
static inline uint32_t
gb_buf_size(struct gapbuf *gb)
{
	return gb->ebuf - gb->buf;
}

/**
 * Return the gap size in bytes
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Number of bytes in the gap.
 */
static inline uint32_t
gb_gap_size(struct gapbuf *gb)
{
	return gb->egap - gb->gap;
}

/**
 * Number of data bytes
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Number of data bytes
 */
static inline uint32_t
gb_data_size(struct gapbuf *gb)
{
	return (gb->ebuf - gb->buf) - (gb->egap - gb->gap);
}

/**
 * Return the start of the buffer address
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   The pointer to the start of the buffer
 */
static inline char *
gb_start_of_buf(struct gapbuf *gb)
{
	return gb->buf;
}

/**
 * Return the pointer to the gap start
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Pointer to the gap start location
 */
static inline char *
gb_start_of_gap(struct gapbuf *gb)
{
	return gb->gap;
}

/**
 * Return the pointer to the end of the gap
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   The end of the gap pointer
 */
static inline char *
gb_end_of_gap(struct gapbuf *gb)
{
	return gb->egap;
}

/**
 * Return the pointer to the end of the buffer
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   End of buffer pointer
 */
static inline char *
gb_end_of_buf(struct gapbuf *gb)
{
	return gb->ebuf;
}

/**
 * Return the point location
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Pointer to point
 */
static inline char *
gb_point_at(struct gapbuf *gb)
{
	return gb->point;
}

/**
 * Is point at start of buffer
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   true if point is at start of buffer
 */
static inline int
gb_point_at_start(struct gapbuf *gb)
{
	return (gb->point == gb->buf);
}

/**
 * is point at the end of buffer
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   true if the point is at the end of buffer
 */
static inline int
gb_point_at_end(struct gapbuf *gb)
{
	return (gb->ebuf == gb->point);
}

/**
 * is point at start of gap
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   true if the point is at the gap start
 */
static inline int
gb_point_at_gap(struct gapbuf *gb)
{
	return (gb->gap == gb->point);
}

/**
 * Set point to a givewn index into the buffer
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param idx
 *   Index into the buffer to put point
 * @return
 *   N/A
 */
static inline void
gb_set_point(struct gapbuf *gb, int idx)
{
	if (idx == -1) {
		gb->point = gb->ebuf;
		return;
	}
	gb->point = gb->buf + idx;
	if (gb->point > gb->gap)
		gb->point += gb->egap - gb->gap;
}

/**
 * Get offset of point
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Offset of point from start of buffer
 */
static inline int
gb_point_offset(struct gapbuf *gb)
{
	if (gb->point > gb->egap)
		return (gb->point - gb->buf) - (gb->egap - gb->gap);
	else
		return gb->point - gb->buf;
}

/**
 * Return true if point is at end of buffer data.
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   True if end of buffer data
 */
static inline int
gb_eof(struct gapbuf *gb)
{
	return (gb->point == gb->gap)?
	       (gb->egap == gb->ebuf) : (gb->point == gb->ebuf);
}

/********************************************************/

/**
 * Move the gap to the location of the point.
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   N/A
 */
static inline void
gb_move_gap_to_point(struct gapbuf *gb)
{
	if (gb->point == gb->gap)
		return;

	if (gb->point == gb->egap)
		gb->point = gb->gap;
	else {
		int cnt;

		if (gb->point < gb->gap) {
			cnt = gb->gap - gb->point;
			memmove(gb->egap - cnt, gb->point, cnt);
			gb->egap -= cnt;
			gb->gap = gb->point;
		} else if (gb->point > gb->egap) {
			cnt = gb->point - gb->egap;
			memmove(gb->gap, gb->egap, cnt);
			gb->gap += cnt;
			gb->egap = gb->point;
			gb->point = gb->gap;
		} else {    /* This case when point is between gap and egap. */
			cnt = gb->point - gb->gap;
			memmove(gb->gap, gb->egap, cnt);
			gb->egap += cnt;
			gb->gap += cnt;
			gb->point = gb->gap;
		}
	}
}

/**
 * Expand the buffer by the given bytes.
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param more
 *   The number of bytes to increase the buffer
 * @return
 *   N/A
 */
static inline void
gb_expand_buf(struct gapbuf *gb, uint32_t more)
{
	if (((gb->ebuf - gb->buf) + more)  > gb_buf_size(gb)) {
		char *old = gb->buf;

		more = (gb->ebuf - gb->buf) + more + GB_DEFAULT_GAP_SIZE;

		gb->buf = (char *)realloc(gb->buf, more);
		if (gb->buf == NULL)
			rte_panic("realloc(%d) in %s failed\n", more, __func__);

		gb->point   += (gb->buf - old);
		gb->ebuf    += (gb->buf - old);
		gb->gap     += (gb->buf - old);
		gb->egap    += (gb->buf - old);
	}
}

/**
 * Expand the Gap by the size given.
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param size
 *   Increase the gap by the number of bytes given
 * @return
 *   N/A
 */
static inline void
gb_expand_gap(struct gapbuf *gb, uint32_t size)
{
	if (size > gb_gap_size(gb)) {
		size += GB_DEFAULT_GAP_SIZE;

		gb_expand_buf(gb, size);

		memmove(gb->egap + size, gb->egap, gb->ebuf - gb->egap);

		gb->egap += size;
		gb->ebuf += size;
	}
}

/**
 * Get the byte at the point location.
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Byte at point
 */
static inline char
gb_get(struct gapbuf *gb)
{
	if (gb->point == gb->gap)
		gb->point = gb->egap;

	return *gb->point;
}

/**
 * Get the byte at the point - 1 location.
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Byte at point
 */
static inline char
gb_get_prev(struct gapbuf *gb)
{
	if (gb->point == gb->egap)
		gb->point = gb->gap;

	if (gb->point == gb->buf) {
		if (gb->point == gb->gap)
			return '\0';
		else
			return *gb->point;
	}

	return *(gb->point - 1);
}

/**
 * Get the byte at the point + 1 location.
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Byte at point
 */
static inline char
gb_get_next(struct gapbuf *gb)
{
	if (gb->point == gb->gap)
		gb->point = gb->egap;

	if (gb->point == gb->ebuf)
		return *gb->point;

	return *(gb->point + 1);
}

/**
 * Put character at point
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param c
 *   The character to put at point
 * @return
 *   N/A
 */
static inline void
gb_put(struct gapbuf *gb, char c)
{
	if (gb->point == gb->gap)
		gb->point = gb->egap;

	if (gb->point == gb->ebuf) {
		gb_expand_buf(gb, 1);
		gb->ebuf++;
	}

	*gb->point = c;
}

/**
 * Get the byte at the point location and advance point
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Byte at point
 */
static inline char
gb_getc(struct gapbuf *gb)
{
	if (gb->point == gb->gap) {
		gb->point = gb->egap + 1;
		return *gb->egap;
	}

	return *(gb->point++);
}

/**
 * Move point left and return character at point.
 *
 * fmrgetc() (point == ehole ? *(point = hole - 1) : *(--point))
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Character at point
 */
static inline char
gb_getc_prev(struct gapbuf *gb)
{
	if (gb->point == gb->egap)
		gb->point = gb->gap;

	return *(--gb->point);
}

/**
 * Put character at point and advance point
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param c
 *   The character to put at point
 * @return
 *   N/A
 */
static inline void
gb_putc(struct gapbuf *gb, char c)
{
	gb_move_gap_to_point(gb);

	if (gb->point == gb->ebuf) {
		gb_expand_buf(gb, 1);
		gb->ebuf++;
	}
	*(gb->gap++) = c;
	gb->point++;
}

/**
 * Insert the character at point and move point.
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param c
 *   The character to add to buffer
 * @return
 *   N/A
 */
static inline void
gb_insert(struct gapbuf *gb, char c)
{
	if (gb->point != gb->gap)
		gb_move_gap_to_point(gb);

	if (gb->gap == gb->egap)
		gb_expand_gap(gb, 1);

	gb_putc(gb, c);
}

/**
 * Delete the character(s) at point
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param cnt
 *   Number of characters to delete at point.
 * @return
 *   N/A
 */
static inline void
gb_del(struct gapbuf *gb, int cnt)
{
	if (gb->point != gb->gap)
		gb_move_gap_to_point(gb);

	gb->egap += cnt;
}

/**
 * Insert a string at point and move point
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @param str
 *   String to put at point
 * @param
 *   Size of the string to insert at point, if zero use strlen() to find length
 * @return
 *   Number of bytes inserted
 */
static inline uint32_t
gb_str_insert(struct gapbuf *gb, char *str, uint32_t size)
{
	int len;

	if (size == 0)
		size = strlen(str);
	if (size == 0)
		return 0;

	gb_move_gap_to_point(gb);

	if (size > gb_gap_size(gb))
		gb_expand_gap(gb, size);

	len = size;
	do {
		gb_putc(gb, *str++);
	} while(--size);

	return len;
}

/********************************************************/

/**
 * Left size of the data in the gap buffer
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Number of bytes in the left size of gap buffer
 */
static inline uint32_t
gb_left_data_size(struct gapbuf *gb)
{
	return gb->gap - gb->buf;
}

/**
 * Right size of the data in the gap buffer
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   Number of bytes in the right size of gap buffer
 */
static inline uint32_t
gb_right_data_size(struct gapbuf *gb)
{
	if (gb_eof(gb))
		return 0;
	return gb->ebuf - gb->egap;
}

/**
 * Move point right one byte
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   N/A
 */
static inline void
gb_move_right(struct gapbuf *gb)
{
	if (gb->point == gb->gap)
		gb->point = gb->egap;
	gb->point = ((gb->point + 1) > gb->ebuf)? gb->ebuf : (gb->point + 1);
}

/**
 * Move point left one byte
 *
 * @param gb
 *   The gapbuf structure pointer.
 * @return
 *   N/A
 */
static inline void
gb_move_left(struct gapbuf *gb)
{
	if (gb->point == gb->egap)
		gb->point = gb->gap;
	gb->point = ((gb->point - 1) < gb->buf)? gb->buf : (gb->point - 1);
}

#ifdef __cplusplus
}
#endif

#endif /* _CLI_GAPBUF_H_ */
