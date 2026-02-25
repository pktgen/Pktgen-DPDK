/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_DISPLAY_H_
#define _PKTGEN_DISPLAY_H_

/**
 * @file
 *
 * Terminal display helpers for Pktgen: screen initialisation, top/dash
 * line drawing, geometry queries, colour-theme management, and column
 * divider rendering.
 */

/* TODO create pktgen_display_*() abstractions and remove this #include */
#include <cli_scrn.h>

#include <copyright_info.h>

#include "pktgen.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialise the terminal screen and colour theme.
 *
 * @param theme
 *   Non-zero to enable the colour theme; 0 for monochrome output.
 */
void pktgen_init_screen(int theme);

/**
 * Print the top banner line of the display with page title and port range.
 *
 * @param msg
 *   Page title string to centre in the top line.
 * @param pstart
 *   First port index included on this display page.
 * @param pstop
 *   Last port index included on this display page (inclusive).
 * @param pcnt
 *   Total number of ports shown on this page.
 */
void display_topline(const char *msg, int pstart, int pstop, int pcnt);

/**
 * Print a full-width dashed separator line at the given row.
 *
 * @param last_row
 *   Terminal row number at which to draw the dashed line.
 */
void display_dashline(int last_row);

/**
 * Query the current terminal geometry.
 *
 * @param rows
 *   Output: number of terminal rows.
 * @param cols
 *   Output: number of terminal columns.
 */
void pktgen_display_get_geometry(uint16_t *rows, uint16_t *cols);

/**
 * Apply the foreground/background colours associated with theme element @p elem.
 *
 * @param elem
 *   Theme element name string (e.g. "top.page", "stats.total.label").
 */
void pktgen_display_set_color(const char *elem);

/**
 * Update the CLI prompt string to reflect the current colour theme.
 *
 * When the colour theme is enabled the prompt includes ANSI escape codes;
 * when disabled it is plain text.
 */
void pktgen_set_prompt(void);

/**
 * Print all defined colour theme entries with their associated colours.
 */
void pktgen_show_theme(void);

/**
 * Set the colours and text attribute for a named theme item.
 *
 * @param item
 *   Theme item name to update.
 * @param fg_color
 *   Foreground colour name string (e.g. "red", "default").
 * @param bg_color
 *   Background colour name string.
 * @param attr
 *   Text attribute string (e.g. "bold", "underline", "none").
 */
void pktgen_set_theme_item(char *item, char *fg_color, char *bg_color, char *attr);

/**
 * Save the current colour theme as a sequence of Pktgen commands to @p filename.
 *
 * @param filename
 *   Path to the file in which the theme commands are written.
 */
void pktgen_theme_save(char *filename);

/**
 * Enable or disable the colour theme.
 *
 * @param state
 *   "on" to enable the colour theme, "off" to disable it.
 */
void pktgen_theme_state(const char *state);

/**
 * Print the current colour-theme enable/disable state to the console.
 */
void pktgen_theme_show(void);

/**
 * Draw a vertical column divider (colon characters) between two rows.
 *
 * @param row_first
 *   First terminal row at which to start drawing the divider.
 * @param row_last
 *   Last terminal row at which to stop drawing the divider (inclusive).
 * @param col
 *   Terminal column at which the divider is drawn.
 */
void pktgen_print_div(uint32_t row_first, uint32_t row_last, uint32_t col);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_DISPLAY_H_ */
