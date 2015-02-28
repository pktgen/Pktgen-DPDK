/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef __INCLUDE_WR_SCRN_H_
#define __INCLUDE_WR_SCRN_H_

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

#include <rte_atomic.h>

/** scrn version number */
#define SCRN_VERSION	"1.3.0"

/** Structure to hold information about the screen and control access. */
typedef struct wr_scrn_s {
	rte_atomic32_t	pause;		/**< Pause the update of the screen. */
	rte_atomic32_t	state;		/**< Screen state on or off */
	uint16_t		nrows;		/**< Max number of rows. */
	uint16_t		ncols;		/**< Max number of columns. */
	uint16_t		theme;		/**< Current theme state on or off */
	uint16_t		pad0;		/**< alignment */
} wr_scrn_t;

extern wr_scrn_t	* __scrn;	/**< Global extern for wr_scrn_t pointer (their can be only one!) */

/** Enable or disable the screen from being updated */
enum { SCRN_RUNNING = 0, SCRN_PAUSED = 1 };

/** Enable or disable the theme or color options */
enum { THEME_OFF = 0, THEME_ON = 1 };

/** ANSI color codes zero based, need to add 30 or 40 for foreground or background color code */
typedef enum { BLACK = 0, RED = 1, GREEN = 2, YELLOW = 3, BLUE = 4, MAGENTA = 5, CYAN = 6, WHITE = 7,
			   RGB = 8, DEFAULT_FG = 9, DEFAULT_BG = 9, NO_CHANGE = 98, UNKNOWN_COLOR=99 } color_e;

/** ANSI color codes zero based for attributes per color */
typedef enum { OFF = 0, BOLD = 1, UNDERSCORE = 4, BLINK = 5, REVERSE = 7, CONCEALED = 8, NO_ATTR = 98, UNKNOWN_ATTR = 99 } attr_e;

/** A single byte to hold port of a Red/Green/Blue color value */
typedef uint8_t		rgb_t;

/** Macro to reduce typing and screen clutter */
#define	wr_scrn_puts(...)	{ printf(__VA_ARGS__); fflush(stdout); }

/** The following inline routines to output ANSI escape codes */
static __inline__ void wr_scrn_pos(int r, int c)	wr_scrn_puts("\033[%d;%dH", r, c)	/** position cursor to row and column */
static __inline__ void wr_scrn_top(void)			wr_scrn_puts("\033H")				/** Move cursor to the top left of the screen */
static __inline__ void wr_scrn_home(void)			wr_scrn_puts("\033H")				/** Move cursor to the Home position */
static __inline__ void wr_scrn_coff(void)			wr_scrn_puts("\033[?25l")			/** Turn cursor off */
static __inline__ void wr_scrn_con(void)			wr_scrn_puts("\033[?25h")			/** Turn cursor on */
static __inline__ void wr_scrn_turn_on(void)		wr_scrn_puts("\033[?25h")			/** Hide cursor */
static __inline__ void wr_scrn_turn_off(void)		wr_scrn_puts("\033[?25l")			/** Display cursor */
static __inline__ void wr_scrn_save(void)			wr_scrn_puts("\0337")				/** Save current cursor position */
static __inline__ void wr_scrn_restore(void)		wr_scrn_puts("\0338")				/** Restore the saved cursor position */
static __inline__ void wr_scrn_eol(void)			wr_scrn_puts("\033[K")				/** Clear from cursor to end of line */
static __inline__ void wr_scrn_cbl(void)			wr_scrn_puts("\033[1K")				/** Clear from cursor to begining of line */
static __inline__ void wr_scrn_cel(void)			wr_scrn_puts("\033[2K")				/** Clear entire line */
static __inline__ void wr_scrn_clw(void)			wr_scrn_puts("\033[J")				/** Clear from cursor to end of screen */
static __inline__ void wr_scrn_clb(void)			wr_scrn_puts("\033[1J")				/** Clear from cursor to begining of screen */
static __inline__ void wr_scrn_cls(void)			wr_scrn_puts("\033[2J")				/** Clear the screen, more cursor to home */
static __inline__ void wr_scrn_reverse(void)		wr_scrn_puts("\033[7m")				/** Start reverse video */
static __inline__ void wr_scrn_normal(void)		wr_scrn_puts("\033[0m")					/** Stop attribute like reverse and underscore */
static __inline__ void wr_scrn_scroll(int r)		wr_scrn_puts("\033[%d;r",r)			/** Scroll whole screen up r number of lines */
static __inline__ void wr_scrn_scroll_up(int r)	wr_scrn_puts("\033[%dS",r)				/** Scroll whole screen up r number of lines */
static __inline__ void wr_scrn_scroll_down(int r)	wr_scrn_puts("\033[%dT",r)			/** Scroll whole screen down r number of lines */
static __inline__ void wr_scrn_nlines(int r)		wr_scrn_puts("\033[%dE",r)			/** Move down nlines plus move to column 1 */
static __inline__ void wr_scrn_setw(int t)			wr_scrn_puts("\033[%d;r", t)		/** Set window size, from to end of screen */
static __inline__ void wr_scrn_cpos(void)			wr_scrn_puts("\0336n")				/** Cursor postion report */

/** Return the version string */
static __inline__ const char * wr_scrn_version(void) {
	return SCRN_VERSION;
}

/** Position the cursor to a line and clear the entire line */
static __inline__ void wr_scrn_clr_line(int r)	{
	wr_scrn_pos(r, 0);
	wr_scrn_cel();
}

/** Position cursor to row/column and clear to end of line */
static __inline__ void wr_scrn_eol_pos(int r, int c) {
	wr_scrn_pos(r, c);
	wr_scrn_eol();
}

extern void __set_prompt(void);

/** Stop screen from updating until resumed later */
static __inline__ void wr_scrn_pause(void) {
	rte_atomic32_set(&__scrn->pause, SCRN_PAUSED);
	__set_prompt();
}

/** Resume the screen from a pause */
static __inline__ void wr_scrn_resume(void) {
	rte_atomic32_set(&__scrn->pause, SCRN_RUNNING);
	__set_prompt();
}

/* Is the screen in the paused state */
static __inline__ int wr_scrn_is_paused(void) {
	return (rte_atomic32_read(&__scrn->pause) == SCRN_PAUSED);
}

/** Output a message of the current line centered */
static __inline__ int wr_scrn_center_col(int16_t ncols, const char * msg) {
	int16_t		s;

	s = ((ncols/2) - (strlen(msg)/2));
	return (s <= 0)? 1 : s;
}

/** Erase the screen by scrolling it off the display, then put cursor at the bottom */
static __inline__ void wr_scrn_erase(int16_t nrows) {
	int16_t		i, cnt;
	const char * nl = "\n\n\n\n\n\n\n\n";

	wr_scrn_setw(1);				// Clear the window to full screen.
	wr_scrn_pos(nrows+1, 1);		// Put cursor on the last row.

	// Scroll the screen to clear the screen and keep the previous information in scrollbar.
	for(i = 0, cnt = 0; i < (nrows/(int16_t)strlen(nl)); i++, cnt += strlen(nl))
		printf("%s", nl);

	// Scroll the last set of rows.
	for(i = cnt; i < nrows; i++)
		printf("\n");

	fflush(stdout);
}

/** Output a string at a row/column for a number of times */
static __inline__ void wr_scrn_repeat(int16_t r, int16_t c, const char * str, int cnt) {
	int		i;

	wr_scrn_pos(r, c);
	for(i=0; i<cnt; i++)
		printf("%s", str);

	fflush(stdout);
}

/** Output a column of strings at a given starting row for a given number of times */
static __inline__ void wr_scrn_col_repeat(int16_t r, int16_t c, const char * str, int cnt) {
	int		i;

	for(i=0; i<cnt; i++) {
		wr_scrn_pos(r++, c);
		printf("%s", str);
	}
}

/** Set the foreground color + attribute at the current cursor position */
static __inline__ void wr_scrn_fgcolor( color_e color, attr_e attr ) {
    wr_scrn_puts("\033[%d;%dm", attr, color + 30);
}

/** Set the background color + attribute at the current cursor position */
static __inline__ void wr_scrn_bgcolor( color_e color, attr_e attr ) {
    wr_scrn_puts("\033[%d;%dm", attr, color + 40);
}

/** Set the foreground/background color + attribute at the current cursor position */
static __inline__ void wr_scrn_fgbgcolor( color_e fg, color_e bg, attr_e attr ) {
		wr_scrn_puts("\033[%d;%d;%dm", attr, fg + 30, bg + 40);
}

/** Main routine to set color for foreground and background nd attribute at the current position */
static __inline__ void wr_scrn_color(color_e fg, color_e bg, attr_e attr) {

	if ( (fg != NO_CHANGE) && (bg != NO_CHANGE) )
		wr_scrn_fgbgcolor(fg, bg, attr);
	else if ( fg == NO_CHANGE )
		wr_scrn_bgcolor(bg, attr);
	else if ( bg == NO_CHANGE )
		wr_scrn_fgcolor(fg, attr);
}

/** Setup for 256 RGB color methods. A routine to output RGB color codes if supported */
static __inline__ void wr_scrn_rgb(uint8_t fg_bg, rgb_t r, rgb_t g, rgb_t b) {
	wr_scrn_puts("\033[%d;2;%d;%d;%dm", fg_bg, r, g, b);
}

/** External functions used for positioning the cursor and outputing a string like printf */
extern wr_scrn_t * wr_scrn_init(int16_t nrows, int16_t ncols, int theme);

extern void wr_scrn_center(int16_t r, int16_t ncols, const char * fmt, ...);
extern void wr_scrn_printf(int16_t r, int16_t c, const char * fmt, ...);
extern void wr_scrn_fprintf(int16_t r, int16_t c, FILE * f, const char * fmt, ...);

#define rte_printf_status(...)	wr_scrn_fprintf(0, 0, stdout, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* _INCLUDE_WR_SCRN_H_ */
