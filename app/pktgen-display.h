/*-
 * Copyright (c) <2010>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

#ifndef _PKTGEN_DISPLAY_H_
#define _PKTGEN_DISPLAY_H_

/* TODO create pktgen_display_*() abstractions and remove this #include */
#include <wr_scrn.h>

#include <wr_copyright_info.h>

#include "pktgen.h"


/* Initialize screen data structures */
extern void pktgen_init_screen(int theme);


/**************************************************************************//**
*
* display_topline - Print out the top line on the screen.
*
* DESCRIPTION
* Print out the top line on the screen and any other information.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void display_topline(const char * msg);


/**************************************************************************//**
*
* display_dashline - Print out the dashed line on the screen.
*
* DESCRIPTION
* Print out the dashed line on the screen and any other information.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void display_dashline(int last_row);


/**************************************************************************//**
*
* pktgen_display_set_geometry - Set the display geometry
*
* DESCRIPTION
* Set the display geometry.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_display_set_geometry(uint16_t rows, uint16_t cols);


/**************************************************************************//**
*
* pktgen_display_get_geometry - Get the display geometry
*
* DESCRIPTION
* Get the display geometry.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_display_get_geometry(uint16_t *rows, uint16_t *cols);


/**************************************************************************//**
*
* pktgen_display_set_color - Changes the color to the color of the specified element.
*
* DESCRIPTION
* Changes the color to the color of the specified element.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_display_set_color(const char *elem);


/**************************************************************************//**
*
* pktgen_set_prompt - Sets the prompt for the command line.
* The new string will include color support if enabled, which includes
* ANSI color codes to style the prompt according to the color theme.
*
* DESCRIPTION
* None
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_set_prompt(void);

/**************************************************************************//**
*
* pktgen_show_theme - Display the current color theme information
*
* DESCRIPTION
* Display the current color theme information with color
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_show_theme(void);

/**************************************************************************//**
*
* pktgen_set_theme_item - Set the given item name with the colors and attribute
*
* DESCRIPTION
* Set the given theme item with the colors and attributes.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_set_theme_item( char * item, char * fg_color, char * bg_color, char * attr);

/**************************************************************************//**
*
* pktgen_theme_save - Save the theme to a file.
*
* DESCRIPTION
* Save a set of commands to set the theme colors and attributes.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_theme_save(char * filename);

/**************************************************************************//**
*
* pktgen_theme_state - Set the current theme state.
*
* DESCRIPTION
* Set the current theme state.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_theme_state(const char * state);

/**************************************************************************//**
*
* pktgen_theme_show - Show the current theme state.
*
* DESCRIPTION
* Show the current theme state.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_theme_show(void);

#endif	/* _PKTGEN_DISPLAY_H_ */


