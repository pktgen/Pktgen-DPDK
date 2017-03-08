/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2016-2017 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
}
