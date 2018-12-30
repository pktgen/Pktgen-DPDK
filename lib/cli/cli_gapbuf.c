/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */
/* inspired by an email/code written by: Joseph H. Allen, 9/10/89 */

#include <rte_memcpy.h>

#include "cli.h"

/* Copy the gap buffer data into a user supplied buffer */
uint32_t
gb_copy_to_buf(struct gapbuf *gb, char *dst, uint32_t size)
{
	uint32_t cnt, tcnt = 0;

	RTE_ASSERT(gb != NULL);
	RTE_ASSERT(dst != NULL);

	/* Only copy the request size or data size */
	size = RTE_MIN(size, gb_data_size(gb));

	if (size) {
		/* Move data before the gap */
		cnt = gb->gap - gb->buf;/* Could be zero */
		rte_memcpy(dst, gb->buf, cnt);
		dst += cnt;
		tcnt += cnt;

		/* Move data after the gap */
		cnt = gb->ebuf - gb->egap;	/* Could be zero */
		rte_memcpy(dst, gb->egap, cnt);
		dst += cnt;
		tcnt += cnt;
	}

	/* Force a NULL terminated string */
	*dst = '\0';

	return tcnt;
}

int
gb_reset_buf(struct gapbuf *gb)
{
	int size;

	RTE_ASSERT(gb != NULL);

	size = gb_buf_size(gb);
	memset(gb->buf, ' ', size);

	gb->point   = gb->buf;
	gb->gap     = gb->buf;
	gb->egap    = gb->buf + size;
	gb->ebuf    = gb->egap;

	return 0;
}

/* release the buffer and allocate a new buffer with correct offsets */
int
gb_init_buf(struct gapbuf *gb, int size)
{
	RTE_ASSERT(gb != NULL);

	free(gb->buf);

	gb->buf = malloc(size);
	if (!gb->buf)
		return -1;

	gb->ebuf = gb->buf + size;

	return gb_reset_buf(gb);
}

/* Create the gap buffer structure and init the pointers */
struct gapbuf *
gb_create(void)
{
	struct gapbuf *gb;

	gb = malloc(sizeof(struct gapbuf));
	if (!gb)
		return NULL;

	memset(gb, '\0', sizeof(struct gapbuf));

	gb_init_buf(gb, GB_DEFAULT_GAP_SIZE);

	return gb;
}

/* Release the gap buffer data and memory */
void
gb_destroy(struct gapbuf *gb)
{
	if (gb) {
		free(gb->buf);
		free(gb);
	}
}

/* Dump out the gap buffer and pointers in a readable format */
void
gb_dump(struct gapbuf *gb, const char *msg)
{
#ifdef CLI_DEBUG_ENABLED
	char *p;
	uint32_t i;

	if (msg)
		fprintf(stderr, "\n%s Gap: buf_size %u, gap_size %u\n",
		        msg, gb_buf_size(gb), gb_gap_size(gb));
	else
		fprintf(stderr, "\nGap: buf_size %u, gap_size %u\n",
		        gb_buf_size(gb), gb_gap_size(gb));

	fprintf(stderr, "     buf   %p, ", gb->buf);
	fprintf(stderr, "gap   %p, ", gb->gap);
	fprintf(stderr, "point %p, ", gb->point);
	fprintf(stderr, "egap  %p, ", gb->egap);
	fprintf(stderr, "ebuf  %p\n", gb->ebuf);

	fprintf(stderr, " ");
	for (i = 0, p = gb->buf; p < gb->ebuf; i++, p++)
		fprintf(stderr, "%c", "0123456789"[i % 10]);
	fprintf(stderr, "\n");
	fprintf(stderr, "<");
	for (p = gb->buf; p < gb->ebuf; p++)
		fprintf(stderr, "%c", ((*p >= ' ') && (*p <= '~')) ? *p : '.');
	fprintf(stderr, ">\n ");
	for (p = gb->buf; p <= gb->ebuf; p++) {
		if ((p == gb->gap) && (p == gb->egap))
			fprintf(stderr, "*");
		else if (p == gb->gap)
			fprintf(stderr, "[");
		else if (p == gb->egap)
			fprintf(stderr, "]");
		else
			fprintf(stderr, " ");
	}
	fprintf(stderr, "\n ");
	for (p = gb->buf; p <= gb->ebuf; p++)
		fprintf(stderr, "%c", (p == gb->point) ? '^' : ' ');
	fprintf(stderr, "\n");
	cli_redisplay_line();
#else
	(void)gb;
	(void)msg;
#endif
}
