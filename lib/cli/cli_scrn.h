/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2015-2017 Intel Corporation. All rights reserved.
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

/* Created 2017 by Keith Wiles @ intel.com */

#ifndef __CLI_SCRN_H_
#define __CLI_SCRN_H_

#include <termios.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * RTE simple cursor and color support for VT100 using ANSI color escape codes.
 *
 ***/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <rte_atomic.h>
#include <rte_per_lcore.h>

/** scrn version number */
#define SCRN_VERSION    "2.0.0"

/* Add more defines for new types */
#define SCRN_STDIN_TYPE		0

#define SCRN_DEFAULT_ROWS			44
#define SCRN_DEFAULT_COLS			132

/** Structure to hold information about the screen and control access. */
struct cli_scrn {
    rte_atomic32_t  pause;      /**< Pause the update of the screen. */
    rte_atomic32_t  state;      /**< Screen state on or off */
    uint16_t        nrows;      /**< Max number of rows. */
    uint16_t        ncols;      /**< Max number of columns. */
    uint16_t        theme;      /**< Current theme state on or off */
    uint16_t 		type;       /**< screen I/O type */
    struct termios oldterm;     /**< Old terminal setup information */
    FILE *fd_out;               /**< File descriptor for output */
    FILE *fd_in;                /**< File descriptor for input */
};

RTE_DECLARE_PER_LCORE(struct cli_scrn *, scrn);
#define this_scrn		RTE_PER_LCORE(scrn)

/** Enable or disable the screen from being updated */
enum { SCRN_SCRN_RUNNING = 0, SCRN_SCRN_PAUSED = 1 };

/** Enable or disable the theme or color options */
enum { SCRN_THEME_OFF = 0, SCRN_THEME_ON = 1 };

/** ANSI color codes zero based, need to add 30 or 40 for foreground or
   background color code */
typedef enum {
	SCRN_BLACK = 0, SCRN_RED = 1, SCRN_GREEN = 2, SCRN_YELLOW = 3,
	SCRN_BLUE = 4, SCRN_MAGENTA = 5, SCRN_CYAN = 6,SCRN_WHITE = 7,
	SCRN_RGB = 8, SCRN_DEFAULT_FG = 9, SCRN_DEFAULT_BG = 9,
	SCRN_NO_CHANGE = 98, SCRN_UNKNOWN_COLOR    = 99
} scrn_color_e;

/** ANSI color codes zero based for attributes per color */
typedef enum {
	SCRN_OFF = 0, SCRN_BOLD = 1, SCRN_UNDERSCORE = 4, SCRN_BLINK = 5,
	SCRN_REVERSE = 7, SCRN_CONCEALED = 8, SCRN_NO_ATTR = 98,
	SCRN_UNKNOWN_ATTR = 99
} scrn_attr_e;

/** A single byte to hold port of a Red/Green/Blue color value */
typedef uint8_t cli_rgb_t;

static inline void
scrn_puts(const char *fmt, ...)
{
	struct cli_scrn *scrn = this_scrn;
	FILE * f;
    va_list vaList;

	f = (!scrn || !scrn->fd_out)? stdout : scrn->fd_out;
    va_start(vaList, fmt);
    vfprintf(f, fmt, vaList);
    va_end(vaList);
    fflush(f);
}

void scrn_cprintf(int16_t r, int16_t ncols, const char *fmt, ...);
void scrn_printf(int16_t r, int16_t c, const char *fmt, ...);
void scrn_fprintf(int16_t r, int16_t c, FILE *f, const char *fmt, ...);

#define _s(_x, _y)	static __inline__ void _x { _y; }

	/** position cursor to row and column */
_s(scrn_pos(int r, int c),  scrn_puts("\033[%d;%dH", r, c))

    /** Move cursor to the top left of the screen */
_s(scrn_top(void), scrn_puts("\033H"))

    /** Move cursor to the Home position */
_s(scrn_home(void), scrn_puts("\033H"))

	/** Turn cursor off */
_s(scrn_coff(void), scrn_puts("\033[?25l"))

	/** Turn cursor on */
_s(scrn_con(void), scrn_puts("\033[?25h"))

	/** Hide cursor */
_s(scrn_turn_on(void), scrn_puts("\033[?25h"))

	/** Display cursor */
_s(scrn_turn_off(void), scrn_puts("\033[?25l"))

    /** Save current cursor position */
_s(scrn_save(void), scrn_puts("\0337"))

    /** Restore the saved cursor position */
_s(scrn_restore(void), scrn_puts("\0338"))

    /** Clear from cursor to end of line */
_s(scrn_eol(void), scrn_puts("\033[K"))

	/** Clear from cursor to begining of line */
_s(scrn_cbl(void), scrn_puts("\033[1K"))

	/** Clear entire line */
_s(scrn_cel(void), scrn_puts("\033[2K"))

    /** Clear from cursor to end of screen */
_s(scrn_clw(void), scrn_puts("\033[J"))

	/** Clear from cursor to begining of screen */
_s(scrn_clb(void), scrn_puts("\033[1J"))

	/** Clear the screen, more cursor to home */
_s(scrn_cls(void), scrn_puts("\033[2J"))

	/** Start reverse video */
_s(scrn_reverse(void), scrn_puts("\033[7m"))

	/** Stop attribute like reverse and underscore */
_s(scrn_normal(void), scrn_puts("\033[0m"))

	/** Scroll whole screen up r number of lines */
_s(scrn_scroll(int r), scrn_puts("\033[%d;r", r))

	/** Scroll whole screen up r number of lines */
_s(scrn_scroll_up(int r), scrn_puts("\033[%dS", r))

	/** Scroll whole screen down r number of lines */
_s(scrn_scroll_down(int r), scrn_puts("\033[%dT", r))

	/** Move down nlines plus move to column 1 */
_s(scrn_nlines(int r), scrn_puts("\033[%dE", r))

	/** Set window size, from to end of screen */
_s(scrn_setw(int t), scrn_puts("\033[%d;r", t))

    /** Cursor postion report */
_s(scrn_cpos(void), scrn_puts("\033[6n"))

/* Report Cursor Position	<ESC>[{ROW};{COLUMN}R
   Generated by the device in response to a Query Cursor Position request;
   reports current cursor position. */

/** Return the version string */
static __inline__ const char *
scrn_version(void) {
    return SCRN_VERSION;
}

/** Position the cursor to a line and clear the entire line */
static __inline__ void
scrn_clr_line(int r)
{
    scrn_pos(r, 0);
    scrn_cel();
}

/** Position cursor to row/column and clear to end of line */
static __inline__ void
scrn_eol_pos(int r, int c)
{
    scrn_pos(r, c);
    scrn_eol();
}

extern void __set_prompt(void);

/** Stop screen from updating until resumed later */
static __inline__ void
scrn_pause(void)
{
    rte_atomic32_set(&this_scrn->pause, SCRN_SCRN_PAUSED);
    __set_prompt();
}

/** Resume the screen from a pause */
static __inline__ void
scrn_resume(void)
{
    rte_atomic32_set(&this_scrn->pause, SCRN_SCRN_RUNNING);
    __set_prompt();
}

/* Is the screen in the paused state */
static __inline__ int
scrn_is_paused(void)
{
    return rte_atomic32_read(&this_scrn->pause) == SCRN_SCRN_PAUSED;
}

/** Output a message of the current line centered */
static __inline__ int
scrn_center_col(int16_t ncols, const char *msg)
{
    int16_t s;

    s = ((ncols / 2) - (strlen(msg) / 2));
    return (s <= 0) ? 1 : s;
}

/** Erase the screen by scrolling it off the display, then put cursor at the
   bottom */
static __inline__ void
scrn_erase(int16_t nrows)
{
    int16_t     i, cnt;
    const char  *nl = "\n\n\n\n\n\n\n\n";

    scrn_setw(1);       /* Clear the window to full */
    /* screen. */
    scrn_pos(nrows + 1, 1);     /* Put cursor on the last row. */

    /* Scroll the screen to clear the screen and keep the previous information */
    /* in scrollbar. */
    for (i = 0, cnt = 0; i < (nrows / (int16_t)strlen(nl)); i++, cnt += strlen(nl))
        scrn_printf(0, 0, "%s", nl);

    /* Scroll the last set of rows. */
    for (i = cnt; i < nrows; i++)
        scrn_printf(0, 0, "\n");
}

/** Output a string at a row/column for a number of times */
static __inline__ void
scrn_repeat(int16_t r, int16_t c, const char *str, int cnt)
{
    int i;

    scrn_pos(r, c);
    for (i = 0; i < cnt; i++)
        scrn_printf(0, 0, "%s", str);
}

/** Output a column of strings at a given starting row for a given number of
   times */
static __inline__ void
scrn_col_repeat(int16_t r, int16_t c, const char *str, int cnt)
{
    int i;

    for (i = 0; i < cnt; i++) {
        scrn_pos(r++, c);
        scrn_printf(0, 0, "%s", str);
    }
}

/** Set the foreground color + attribute at the current cursor position */
static __inline__ void
scrn_fgcolor(scrn_color_e color, scrn_attr_e attr)
{
    scrn_puts("\033[%d;%dm", attr, color + 30);
}

/** Set the background color + attribute at the current cursor position */
static __inline__ void
scrn_bgcolor(scrn_color_e color, scrn_attr_e attr)
{
    scrn_puts("\033[%d;%dm", attr, color + 40);
}

/** Set the foreground/background color + attribute at the current cursor
   position */
static __inline__ void
scrn_fgbgcolor(scrn_color_e fg, scrn_color_e bg, scrn_attr_e attr)
{
    scrn_puts("\033[%d;%d;%dm", attr, fg + 30, bg + 40);
}

/** Main routine to set color for foreground and background nd attribute at the
   current position */
static __inline__ void
scrn_color(scrn_color_e fg, scrn_color_e bg, scrn_attr_e attr)
{

    if ( (fg != SCRN_NO_CHANGE) && (bg != SCRN_NO_CHANGE) )
        scrn_fgbgcolor(fg, bg, attr);
    else if (fg == SCRN_NO_CHANGE)
        scrn_bgcolor(bg, attr);
    else if (bg == SCRN_NO_CHANGE)
        scrn_fgcolor(fg, attr);
}

/** Setup for 256 RGB color methods. A routine to output RGB color codes if
   supported */
static __inline__ void
scrn_rgb(uint8_t fg_bg, cli_rgb_t r, cli_rgb_t g, cli_rgb_t b)
{
    scrn_puts("\033[%d;2;%d;%d;%dm", fg_bg, r, g, b);
}

/** External functions used for positioning the cursor and outputing a string
   like printf */
int scrn_create(int scrn_type, int16_t nrows, int16_t ncols, int theme);
int scrn_create_with_defaults(int theme);
void scrn_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* __CLI_SCRN_H_ */
