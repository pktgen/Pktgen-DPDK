/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_DISPLAY_H_
#define _PKTGEN_DISPLAY_H_

/* TODO create pktgen_display_*() abstractions and remove this #include */
#include <cli_scrn.h>

#include <copyright_info.h>

#include "pktgen.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize screen data structures */
void pktgen_init_screen(int theme);

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
void display_topline(const char *msg);

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
void display_dashline(int last_row);

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
void pktgen_display_set_geometry(uint16_t rows, uint16_t cols);

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
void pktgen_display_get_geometry(uint16_t *rows, uint16_t *cols);

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
void pktgen_display_set_color(const char *elem);

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
void pktgen_set_prompt(void);

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
void pktgen_show_theme(void);

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
void pktgen_set_theme_item(char *item,
				  char *fg_color,
				  char *bg_color,
				  char *attr);

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
void pktgen_theme_save(char *filename);

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
void pktgen_theme_state(const char *state);

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
void pktgen_theme_show(void);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_DISPLAY_H_ */
