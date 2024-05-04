/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2021-2024> Intel Corporation
 */
/* Created using ideas from DPDK tgen */

#include <stdint.h>        // for uint32_t, uint16_t, int32_t, uint8_t
#include <stdbool.h>
#include <netinet/in.h>        // for ntohs, htonl, htons

#include <rte_common.h>
#include <rte_log.h>
#include <pg_strings.h>
#include <rte_mbuf.h>
#include <rte_cycles.h>

#include "fgen.h"

extern int _parse_frame(fgen_t *fg, frame_t *f);

static int
_add_frame(fgen_t *fg, uint16_t idx, const char *name, const char *fstr)
{
    frame_t *f;

    if (idx >= fg->max_frames)
        _ERR_RET("Number of frames exceeds %u\n", fg->max_frames);

    f           = &fg->frames[idx];
    f->data_len = 0;
    f->tsc_off  = 0;

    if (strlen(fstr) <= 0 && strlen(fstr) >= FGEN_MAX_STRING_LENGTH)
        _ERR_RET("String is 0 or too long %ld > %d\n", strlen(fstr), FGEN_MAX_STRING_LENGTH);

    f->name[0] = '\0';
    if (name)
        strlcpy(f->name, name, sizeof(f->name));

    if (strlen(f->name) == 0)
        snprintf(f->name, sizeof(f->name), "Frame-%u", f->fidx);

    f->frame_text = strdup(fstr);
    if (!f->frame_text)
        _ERR_GOTO(leave, "Unable to allocate memory\n");

    if (_parse_frame(fg, f) < 0)
        _ERR_GOTO(leave, "Failed to parse frame\n");

    fg->nb_frames++;

    return 0;
leave:
    if (f) {
        f->name[0] = '\0';
        free(f->frame_text);
        f->frame_text = NULL;
    }
    return -1;
}

int
fgen_add_frame(fgen_t *fg, const char *name, const char *fstr)
{
    if (!fg)
        _ERR_RET("fgen_t pointer is NULL\n");

    if (!fstr)
        _ERR_RET("fgen string is NULL\n");

    if (_add_frame(fg, fg->nb_frames, name, fstr) < 0)
        _ERR_RET("Failed to parse frame\n");

    if (fg->flags & (FGEN_VERBOSE | FGEN_DUMP_DATA))
        printf("\n");

    return 0;
}

static inline void
_prefetch_mbuf_data(struct rte_mbuf *m, uint32_t hdr_len)
{
    uint8_t *pkt_data = rte_pktmbuf_mtod(m, uint8_t *);

    rte_prefetch0_write(pkt_data);
    if (hdr_len > RTE_CACHE_LINE_SIZE)
        rte_prefetch0_write(pkt_data + RTE_CACHE_LINE_SIZE);
}

int
fgen_alloc(fgen_t *fg, int low, int high, struct rte_mbuf **mbufs, uint32_t nb_pkts)
{
    frame_t *f;
    void *pkt_data;
    int pkt_len, begin;
    uint64_t tsc;

    if (unlikely(fg == NULL))
        _ERR_RET("fgen_t pointer is NULL\n");

    if (unlikely(mbufs == NULL))
        _ERR_RET("pktmbuf list is NULL\n");

    if (low < 0)
        low = 0;
    if (high >= fg->nb_frames)
        high = fg->nb_frames - 1;
    if (low > high)
        low = high;
    begin = low;

    uint32_t prefetch = RTE_MIN(nb_pkts, 8U);

    tsc = rte_rdtsc_precise();

    for (uint32_t i = 0; i < prefetch; i++)
        rte_prefetch0_write(rte_pktmbuf_mtod(mbufs[i], char *));

    for (uint32_t i = 0; i < nb_pkts; i++) {
        struct rte_mbuf *m = mbufs[i];

        if ((i + prefetch) < nb_pkts)
            rte_prefetch0_write(rte_pktmbuf_mtod(mbufs[i + prefetch], char *));

        f = &fg->frames[low++];
        if (low >= high)
            low = begin;

        pkt_data = fgen_mtod(f, void *);
        pkt_len  = fgen_data_len(f);

        memcpy(rte_pktmbuf_mtod(m, void *), pkt_data, pkt_len);
        rte_pktmbuf_data_len(m) = pkt_len;

        /* increment tsc by one cycle for each packet, just to have different values in each. */
        if (f->tsc_off)
            *rte_pktmbuf_mtod_offset(m, uint64_t *, f->tsc_off) = tsc++;
    }

    return 0;
}

int
fgen_free(fgen_t *fg, struct rte_mbuf **mbufs, uint32_t nb_pkts)
{
    if (!fg || !mbufs || nb_pkts <= 0)
        _ERR_RET("Invalid arguments\n");

    rte_pktmbuf_free_bulk(mbufs, nb_pkts);

    return 0;
}

fgen_t *
fgen_create(uint16_t max_frames, uint16_t frame_sz, int flags)
{
    fgen_t *fg;
    uint16_t bufsz;

    if (max_frames == 0)
        return NULL;

    if (frame_sz == 0)
        frame_sz = 1;

    bufsz = RTE_ALIGN_CEIL(frame_sz, RTE_CACHE_LINE_SIZE);

    fg = calloc(1, sizeof(fgen_t));
    if (fg) {
        frame_t *f;
        char *b;

        fg->frame_bufsz = bufsz;
        fg->max_frames  = max_frames;
        fg->flags       = flags;

        f = fg->frames = calloc(max_frames, sizeof(frame_t));
        if (!fg->frames)
            _ERR_GOTO(leave, "Unable to allocate frame_t structures\n");

        fg->mm = calloc(max_frames, bufsz);
        if (!fg->mm)
            _ERR_GOTO(leave, "Unable to allocate frame buffers\n");

        b = fg->mm;

        for (int i = 0; i < max_frames; i++, f++) {
            f->fidx  = i;
            f->data  = b;
            f->bufsz = bufsz;
            b += bufsz;
        }
    }

    return fg;
leave:
    fgen_destroy(fg);
    return NULL;
}

void
fgen_destroy(fgen_t *fg)
{
    if (fg) {
        for (int i = 0; i < fg->nb_frames; i++)
            free(fg->frames[i].frame_text);

        free(fg->frames);
        free(fg->mm);
        free(fg);
    }
}

static int
get_frame_string(FILE *f, char *str, int len)
{
    char buf[FGEN_MAX_STRING_LENGTH], *p, *c;
    long pos;
    int slen = 0;

    memset(buf, 0, sizeof(buf));

    /* Here we are gathering up all of the frame text into a single buffer,
     * as the fgen frame text can be split over multiple lines.
     */
    for (;;) {
        pos = ftell(f); /* Save the position to the start of the line */

        if (fgets(buf, sizeof(buf), f) == NULL)
            break;

        /* Trim off any comments in the line, which could leave nothing in the buffer */
        if ((c = strstr(buf, "//")) != NULL)
            *c = '\0';

        p = pg_strtrim(buf); /* remove any leading or trailing whitespace */

        /* empty string continue */
        if (*p == '\0')
            continue;

        /* Check to see if we have a ':=' string in the line, which means we have read too much */
        c = strstr(p, ":=");
        if (c) {
            /* seek backup in the file to the beginning of the line and leave */
            fseek(f, pos, SEEK_SET);
            break;
        }

        /* This line must be part of the current fgen text line, append the string */
        slen = strlcat(str, p, len);

        /* Make sure the layer or line has a trailing '/' */
        if (str[slen - 1] != '/')
            slen = strlcat(str, "/", len);
    }

    /* Strip off the tailing '/' we added to the string */
    if (slen)
        str[--slen] = '\0';

    return slen;
}

/**
 * Find the next frame string, return -1 on EOF, 0 not found, 1 Found
 */
static int
find_next_frame(FILE *f, char *name, int len)
{
    char line[FGEN_MAX_STRING_LENGTH], *p, *c;
    long pos;

    name[0] = '\0';
    for (;;) {
        pos = ftell(f);

        line[0] = '\0';
        if (fgets(line, sizeof(line), f) == NULL)
            return -1;

        c = strstr(line, "//");
        if (c)
            *c = '\0';

        p = pg_strtrim(line);
        if (!p || strlen(p) < 2)
            continue;

        c = strstr(p, ":=");
        if (c == NULL)
            continue;

        if (c != p) { /* Must be a name present */
            *c = '\0';
            strlcpy(name, p, len);
        }

        p = c + 2; /* Skip the ':=' string */
        p = pg_strtrim(p);

        /* Seek back to the start of the frame-string just after ':=' */
        fseek(f, pos + (p - line), SEEK_SET);
        return 1;
    }

    return 0;
}

int
fgen_load_file(fgen_t *fg, const char *filename)
{
    FILE *f;
    char buf[FGEN_MAX_STRING_LENGTH], name[FGEN_FRAME_NAME_LENGTH + 1];
    int ret, cnt;

    if (!fg || !filename || strlen(filename) == 0)
        _ERR_RET("Filename is not specified\n");

    f = fopen(filename, "r");
    if (!f)
        _ERR_RET("Unable to open file '%s'\n", filename);

    memset(name, 0, sizeof(name));
    memset(buf, 0, sizeof(buf));

    for (cnt = 0;; cnt++) {
        name[0] = '\0';
        ret     = find_next_frame(f, name, sizeof(name));
        if (ret <= 0)
            break;

        buf[0] = '\0';
        if (get_frame_string(f, buf, sizeof(buf)) == 0)
            break;

        if (strlen(name) == 0)
            snprintf(name, sizeof(name), "frame-%d", cnt);

        if (_add_frame(fg, cnt, name, buf) < 0)
            _ERR_RET("Adding a frame failed\n");
    }

    return fg->nb_frames;
}

int
fgen_load_strings(fgen_t *fg, const char **fstr, int len)
{
    char *c   = NULL;
    char *s   = NULL;
    char *txt = NULL;
    char name[FGEN_FRAME_NAME_LENGTH];
    int cnt;

    if (!fg)
        _ERR_RET("fgen_t pointer is NULL\n");
    if (!fstr)
        _ERR_RET("Frame string pointer array is NULL\n");

    for (cnt = 0;; cnt++) {
        if (len == 0 && fstr[cnt] == NULL)
            break;

        s = txt = strdup(fstr[cnt]);
        if (!txt)
            _ERR_RET("Unable to strdup() text\n");

        name[0] = '\0';
        if ((c = strstr(s, ":=")) == NULL)
            snprintf(name, sizeof(name), "frame-%d", cnt);
        else {
            *c = '\0';
            strlcpy(name, s, sizeof(name));
            s = c + 2;
            while ((*s == ' ' || *s == '\t' || *s == '\n') && *s != '\0')
                s++;
            if (*s == '\0')
                _ERR_RET("Invalid frame name '%s'\n", fstr[cnt]);
        }

        if (_add_frame(fg, cnt, name, s) < 0)
            _ERR_RET("Adding a frame failed\n");
    }

    free(txt);
    return cnt;
}

void
fgen_print_frame(const char *msg, frame_t *f)
{
    char *layers[FGEN_MAX_LAYERS];
    char *txt;
    int num;

    if (!f->frame_text || strlen(f->frame_text) == 0)
        _RET("text pointer is NULL or zero length string\n");

    printf("\n");
    if (msg)
        printf("[yellow]>>>> [magenta]%s[]: '[orange]%s[]'\n", msg, f->name);
    else
        printf("[yellow]>>>> '[orange]%s[]'\n", f->name);

    txt = strdup(f->frame_text);
    if (!txt)
        _RET("Unable to strdup() text string\n");

    num = pg_strtok(txt, "/", layers, RTE_DIM(layers));
    if (!num) {
        free(txt);
        _RET("Invalid formatted text string\n");
    }

    for (int i = 0; i < num; i++)
        printf("   [orange]%s[]\n", layers[i]);

    free(txt);
}
