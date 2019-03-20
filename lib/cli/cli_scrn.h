/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */

/* Created by Keith Wiles @ intel.com */

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
#include <unistd.h>

#include <rte_atomic.h>
#include <rte_per_lcore.h>

#define VT100_INITIALIZE        -1

#define vt100_open_square       '['
#define vt100_escape            0x1b
#define vt100_del               0x7f
#define ESC			"\033"

/* Key codes */
#define vt100_word_left         ESC "b"
#define vt100_word_right        ESC "f"
#define vt100_suppr             ESC "[3~"
#define vt100_tab               "\011"

/* Action codes for cli_vt100 */
#define vt100_bell              "\007"
#define vt100_bs                "\010"
#define vt100_bs_clear          "\b \b"

/* cursor codes */
#define vt100_cursor_off	ESC "[?25l"
#define vt100_cursor_on		ESC "[?25h"
#define vt100_save_cursor       ESC "7"
#define vt100_restore_cursor    ESC "8"
#define vt100_line_feed		ESC "D"
#define vt100_crnl		ESC "E"
#define vt100_reverse_line_feed	ESC "M"
#define vt100_up_arr            ESC "[A"
#define vt100_down_arr          ESC "[B"
#define vt100_right_arr         ESC "[C"
#define vt100_left_arr          ESC "[D"
#define vt100_up_lines          ESC "[%dA"
#define vt100_down_lines        ESC "[%dB"
#define vt100_right_columns     ESC "[%dC"
#define vt100_left_columns      ESC "[%dD"
#define vt100_home              ESC "[H"

#define vt100_pos               ESC "[%d;%dH"
#define vt100_setw              ESC "[%d;r"
#define vt100_clear_right       ESC "[0K"
#define vt100_clear_left        ESC "[1K"
#define vt100_clear_down        ESC "[0J"
#define vt100_clear_up          ESC "[1J"
#define vt100_clear_line        ESC "[2K"
#define vt100_clear_screen      ESC "[2J"
#define vt100_pos_cursor        ESC "[%d;%dH"
#define vt100_multi_right       ESC "\133%uC"
#define vt100_multi_left        ESC "\133%uD"

/* Result of parsing : it must be synchronized with
 * vt100_commands[] in vt100_keys.c */
enum {
	VT100_INVALID_KEY = 0,
	VT100_KEY_UP_ARR,
	VT100_KEY_DOWN_ARR,
	VT100_KEY_RIGHT_ARR,
	VT100_KEY_LEFT_ARR,
	VT100_KEY_BKSPACE,
	VT100_KEY_RETURN,
	VT100_KEY_CTRL_A,
	VT100_KEY_CTRL_E,
	VT100_KEY_CTRL_K,
	VT100_KEY_CTRL_Y,
	VT100_KEY_CTRL_C,
	VT100_KEY_CTRL_F,
	VT100_KEY_CTRL_B,
	VT100_KEY_SUPPR,
	VT100_KEY_TAB,
	VT100_KEY_CTRL_D,
	VT100_KEY_CTRL_L,
	VT100_KEY_RETURN2,
	VT100_KEY_META_BKSPACE,
	VT100_KEY_WLEFT,
	VT100_KEY_WRIGHT,
	VT100_KEY_CTRL_W,
	VT100_KEY_CTRL_P,
	VT100_KEY_CTRL_N,
	VT100_KEY_META_D,
	VT100_KEY_CTRL_X,
	VT100_MAX_KEYS
};

extern const char *vt100_commands[];

enum vt100_parse_state {
	VT100_INIT,
	VT100_ESCAPE,
	VT100_ESCAPE_CSI,
	VT100_DONE = -1,
	VT100_CONTINUE = -2
};

#define VT100_BUF_SIZE 8
struct cli_vt100 {
	int bufpos;                     /** Current offset into buffer */
	char buf[VT100_BUF_SIZE];       /** cli_vt100 command buffer */
	enum vt100_parse_state state;   /** current cli_vt100 parser state */
};

struct vt100_cmds {
	const char *str;
	void (*func)(void);
};

/** scrn version number */
#define SCRN_VERSION    "2.0.0"

/* Add more defines for new types */
#define SCRN_STDIN_TYPE		0

/** Structure to hold information about the screen and control access. */
struct cli_scrn {
	rte_atomic32_t  pause;      /**< Pause the update of the screen. */
	uint16_t        nrows;      /**< Max number of rows. */
	uint16_t        ncols;      /**< Max number of columns. */
	uint16_t        theme;      /**< Current theme state on or off */
	uint16_t 	type;       /**< screen I/O type */
	struct termios oldterm;     /**< Old terminal setup information */
	FILE *fd_out;               /**< File descriptor for output */
	FILE *fd_in;                /**< File descriptor for input */
};

/** A single byte to hold port of a Red/Green/Blue color value */
typedef uint8_t scrn_rgb_t;

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
	SCRN_BLUE = 4, SCRN_MAGENTA = 5, SCRN_CYAN = 6, SCRN_WHITE = 7,
	SCRN_RGB = 8, SCRN_DEFAULT_FG = 9, SCRN_DEFAULT_BG = 9,
	SCRN_NO_CHANGE = 98, SCRN_UNKNOWN_COLOR    = 99
} scrn_color_e;

/** ANSI color codes zero based for attributes per color */
typedef enum {
	SCRN_OFF = 0, SCRN_BOLD = 1, SCRN_FAINT = 2, SCRN_ITALIC = 3,
	SCRN_UNDERSCORE = 4, SCRN_SLOW_BLINK = 5, SCRN_FAST_BLINK = 6,
	SCRN_REVERSE = 7, SCRN_CONCEALED = 8, SCRN_CROSSOUT = 9,
	SCRN_DEFAULT_FONT = 10, SCRN_UNDERLINE_OFF = 24, SCRN_BLINK_OFF = 25,
	SCRN_INVERSE_OFF = 27, SCRN_REVEAL = 28, SCRN_NOT_CROSSED_OUT = 29,
	/* 30-39 and 40-49 Foreground and Background colors */
	SCRN_FRAMED = 51, SCRN_ENCIRCLED = 52, SCRN_OVERLINED = 53,
	SCRN_NOT_FRAMED = 54, SCRN_NOT_OVERLINED = 55,
	SCRN_NO_ATTR = 98, SCRN_UNKNOWN_ATTR = 99
} scrn_attr_e;

#define SCRN_BLINK	SCRN_SLOW_BLINK

/** A single byte to hold port of a Red/Green/Blue color value */
typedef uint8_t cli_rgb_t;

static inline int
scrn_write(const void *str, int len)
{
	if (len <= 0)
		len = strlen(str);

	if (write(fileno(this_scrn->fd_out), str, len) != len)
		fprintf(stderr, "%s: Write failed\n", __func__);

	return len;
}

static inline int
scrn_read(char *buf, int len)
{
	int n = 0;

	if (!buf || !len)
		return 0;

	while(len--)
		n += read(fileno(this_scrn->fd_in), buf++, 1);
	return n;
}

static inline void
__attribute__((format(printf, 1, 2)))
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
_s(scrn_pos(int r, int c),  scrn_puts(vt100_pos, r, c))

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

/** Cursor move right <n> characters */
_s(scrn_cnright(int n), scrn_puts("\033[%dC", n))

/** Cursor move left <n> characters */
_s(scrn_cnleft(int n), scrn_puts("\033[%dD", n))

/** New line */
_s(scrn_newline(void), scrn_puts("\033[20h"))

/** Move one character right */
_s(scrn_cright(void), scrn_puts("\033[C"))

/** Move one character left */
_s(scrn_cleft(void), scrn_puts("\033[D"))

/** Move cursor to begining of line */
_s(scrn_bol(void), scrn_puts("\r"))

/** Return the version string */
static __inline__ const char *
scrn_version(void)
{
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

void __set_prompt(void);

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
	scrn_setw(1);       /* Clear the window to full screen. */
	scrn_pos(nrows + 1, 1);     /* Put cursor on the last row. */
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

/** Set the foreground color + attribute at the current cursor position */
static __inline__ int
scrn_fgcolor_str(char *str, scrn_color_e color, scrn_attr_e attr)
{
	return snprintf(str, 16, ESC "[%d;%dm", attr, color + 30);
}

/** Set the background color + attribute at the current cursor position */
static __inline__ int
scrn_bgcolor_str(char *str, scrn_color_e color, scrn_attr_e attr)
{
	return snprintf(str, 16, ESC "[%d;%dm", attr, color + 40);
}

/** Set the foreground/background color + attribute at the current cursor position */
static __inline__ int
scrn_fgbgcolor_str(char *str, scrn_color_e fg, scrn_color_e bg, scrn_attr_e attr)
{
	return snprintf(str, 16, ESC "[%d;%d;%dm", attr, fg + 30, bg + 40);
}

/**
 * Main routine to set color for foreground and background and attribute at
 * the current position.
 */
static __inline__ int
scrn_color_str(char *str, scrn_color_e fg, scrn_color_e bg, scrn_attr_e attr)
{
	if ( (fg != SCRN_NO_CHANGE) && (bg != SCRN_NO_CHANGE))
		return scrn_fgbgcolor_str(str, fg, bg, attr);
	else if (fg == SCRN_NO_CHANGE)
		return scrn_bgcolor_str(str, bg, attr);
	else if (bg == SCRN_NO_CHANGE)
		return scrn_fgcolor_str(str, fg, attr);
	else
		return 0;
}

/** Setup for 256 RGB color methods. A routine to output RGB color codes if supported */
static __inline__ int
scrn_rgb_str(char *str, uint8_t fg_bg, scrn_rgb_t r, scrn_rgb_t g, scrn_rgb_t b)
{
	return snprintf(str, 16, ESC "[%d;2;%d;%d;%dm", fg_bg, r, g, b);
}

/** External functions used for positioning the cursor and outputing a string
   like printf */
int scrn_create(int scrn_type, int theme);
int scrn_create_with_defaults(int theme);
void scrn_destroy(void);

/**
 * Create the cli_vt100 structure
 *
 * @return
 * Pointer to cli_vt100 structure or NULL on error
 */
struct cli_vt100 *vt100_setup(void);

/**
 * Destroy the cli_vt100 structure
 *
 * @param
 *  The pointer to the cli_vt100 structure.
 */
void vt100_free(struct cli_vt100 *vt);

/**
 * Input a new character.
 *
 * @param vt
 *   The pointer to the cli_vt100 structure
 * @param c
 *   The character to parse for cli_vt100 commands
 * @return
 *   -1 if the character is not part of a control sequence
 *   -2 if c is not the last char of a control sequence
 *   Else the index in vt100_commands[]
 */
int vt100_parse_input(struct cli_vt100 *vt, uint8_t c);

void vt100_do_cmd(int idx);
struct vt100_cmds *vt100_get_cmds(void);

#ifdef __cplusplus
}
#endif

#endif /* __CLI_SCRN_H_ */
