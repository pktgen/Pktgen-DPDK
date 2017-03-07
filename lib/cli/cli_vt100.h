/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
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

/*
 * Copyright (c) 2009, Olivier MATZ <zer0@droids-corp.org>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _VT100_H_
#define _VT100_H_

#include <stdio.h>
#include <stdint.h>

#include <cli_scrn.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VT100_INITIALIZE        -1

#define vt100_open_square       '['
#define vt100_escape            0x1b
#define vt100_del               0x7f
#define vt100_q_escape			"\033"

/* Key codes */
#define vt100_up_arr            "\033[A"
#define vt100_down_arr          "\033[B"
#define vt100_right_arr         "\033[C"
#define vt100_left_arr          "\033[D"
#define vt100_word_left         "\033b"
#define vt100_word_right        "\033f"
#define vt100_suppr             "\033[3~"
#define vt100_tab               "\011"

/* Action codes for cli_vt100 */
#define vt100_bell              "\007"
#define vt100_bs                "\010"
#define vt100_bs_clear          "\b \b"
#define vt100_crnl              "\012\015"
#define vt100_home              "\033M\033E"

/* cursor codes */
#define vt100_pos               "\033[%d;%dH"
#define vt100_setw              "\033[%d;r"
#define vt100_save_cursor       "\0337"
#define vt100_restore_cursor    "\0338"
#define vt100_clear_right       "\033[0K"
#define vt100_clear_left        "\033[1K"
#define vt100_clear_down        "\033[0J"
#define vt100_clear_up          "\033[1J"
#define vt100_clear_line        "\033[2K"
#define vt100_clear_screen      "\033[2J"
#define vt100_pos_cursor        "\033[%d;%dH"
#define vt100_multi_right       "\033\133%uC"
#define vt100_multi_left        "\033\133%uD"

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

/** A single byte to hold port of a Red/Green/Blue color value */
typedef uint8_t scrn_rgb_t;

/** Macro to reduce typing and screen clutter */
#define cli_puts(...)   scrn_puts(__VA_ARGS__)

/** Set the foreground color + attribute at the current cursor position */
static __inline__ void
vt100_fgcolor(scrn_color_e color, scrn_attr_e attr) {
    cli_puts("\033[%d;%dm", attr, color + 30);
}

/** Set the background color + attribute at the current cursor position */
static __inline__ void
vt100_bgcolor(scrn_color_e color, scrn_attr_e attr) {
    cli_puts("\033[%d;%dm", attr, color + 40);
}

/** Set the foreground/background color + attribute at the current cursor position */
static __inline__ void
vt100_fgbgcolor(scrn_color_e fg, scrn_color_e bg, scrn_attr_e attr) {
    cli_puts("\033[%d;%d;%dm", attr, fg + 30, bg + 40);
}

/**
 * Main routine to set color for foreground and background and attribute at
 * the current position.
 */
static __inline__ void
vt100_color(scrn_color_e fg, scrn_color_e bg, scrn_attr_e attr) {
    if ( (fg != SCRN_NO_CHANGE) && (bg != SCRN_NO_CHANGE))
        vt100_fgbgcolor(fg, bg, attr);
    else if (fg == SCRN_NO_CHANGE)
        vt100_bgcolor(bg, attr);
    else if (bg == SCRN_NO_CHANGE)
        vt100_fgcolor(fg, attr);
}

/** Setup for 256 RGB color methods. A routine to output RGB color codes if supported */
static __inline__ void
vt100_rgb(uint8_t fg_bg, scrn_rgb_t r, scrn_rgb_t g, scrn_rgb_t b) {
    cli_puts("\033[%d;2;%d;%d;%dm", fg_bg, r, g, b);
}

/** Set the foreground color + attribute at the current cursor position */
static __inline__ int
vt100_fgcolor_str(char *str, scrn_color_e color, scrn_attr_e attr) {
    return snprintf(str, 16, "\033[%d;%dm", attr, color + 30);
}

/** Set the background color + attribute at the current cursor position */
static __inline__ int
vt100_bgcolor_str(char *str, scrn_color_e color, scrn_attr_e attr) {
	return snprintf(str, 16, "\033[%d;%dm", attr, color + 40);
}

/** Set the foreground/background color + attribute at the current cursor position */
static __inline__ int
vt100_fgbgcolor_str(char *str, scrn_color_e fg, scrn_color_e bg, scrn_attr_e attr) {
    return snprintf(str, 16, "\033[%d;%d;%dm", attr, fg + 30, bg + 40);
}

/**
 * Main routine to set color for foreground and background and attribute at
 * the current position.
 */
static __inline__ int
vt100_color_str(char *str, scrn_color_e fg, scrn_color_e bg, scrn_attr_e attr) {
    if ( (fg != SCRN_NO_CHANGE) && (bg != SCRN_NO_CHANGE))
        return vt100_fgbgcolor_str(str, fg, bg, attr);
    else if (fg == SCRN_NO_CHANGE)
        return vt100_bgcolor_str(str, bg, attr);
    else if (bg == SCRN_NO_CHANGE)
        return vt100_fgcolor_str(str, fg, attr);
	else
		return 0;
}

/** Setup for 256 RGB color methods. A routine to output RGB color codes if supported */
static __inline__ int
vt100_rgb_str(char *str, uint8_t fg_bg, scrn_rgb_t r, scrn_rgb_t g, scrn_rgb_t b) {
    return snprintf(str, 16, "\033[%d;2;%d;%d;%dm", fg_bg, r, g, b);
}

/**
 * Create the cli_vt100 structure
 *
 * @return
 * Pointer to cli_vt100 structure or NULL on error
 */
struct cli_vt100 *vt100_create(void);

/**
 * Destroy the cli_vt100 structure
 *
 * @param
 *  The pointer to the cli_vt100 structure.
 */
void vt100_destroy(struct cli_vt100 *vt);

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

#ifdef __cplusplus
}
#endif

#endif /* _VT100_H_ */
