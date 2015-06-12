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

#include "pktgen-display.h"
#include "pktgen-cmds.h"

#define MAX_COLOR_NAME_SIZE		64
#define MAX_PROMPT_STRING_SIZE	64

static char prompt_str[MAX_PROMPT_STRING_SIZE] = { 0 };

/* String to color value mapping */
typedef struct string_color_map_s {
	const char	  * name;	/**< Color name */
	color_e		color;		/**< Color value for scrn_{fg,bg}color() */
} string_color_map_t;

string_color_map_t string_color_map[] = {
	{ "black",		BLACK		},
	{ "black",		DEFAULT_FG	},
	{ "red",		RED			},
	{ "green",		GREEN		},
	{ "yellow",		YELLOW		},
	{ "blue",		BLUE		},
	{ "magenta",	MAGENTA		},
	{ "cyan",		CYAN		},
	{ "white",		WHITE		},
	{ "white",		DEFAULT_BG	},
	{ "default",	WHITE		},	/* alias */

	{ "none",		NO_CHANGE	},
	{ "default_fg",	NO_CHANGE	},
	{ "default_bg",	NO_CHANGE	},
	{ NULL, 0 }
};

/* String to attribute mapping */
typedef struct string_attr_map_s {
	const char	  * name;	/**< Attribute name */
	attr_e		attr;	/**< Attribute value for scrn_{fg,bg}color_attr() */
} string_attr_map_t;

string_attr_map_t string_attr_map[] = {
	{ "off",		OFF			},
	{ "default",	OFF			},	/* alias */
	{ "bold",		BOLD		},
	{ "bright",		BOLD		},	/* alias */
	{ "underscore",	UNDERSCORE	},
	{ "underline",	UNDERSCORE	},	/* alias */
	{ "blink",		BLINK		},
	{ "reverse",	REVERSE		},
	{ "concealed",	CONCEALED	},
	{ NULL, 0 }
};

/* Element to color mapping */
typedef struct theme_color_map_s {
	const char		  * name;		/**< Display element name */
	color_e			fg_color;
	color_e			bg_color;
	attr_e			attr;
} theme_color_map_t;

theme_color_map_t theme_color_map[] = {
//	{ "element name",		FG_COLOR,	BG_COLOR,	ATTR	}
	{ "default",			DEFAULT_FG,	DEFAULT_BG,	OFF		},

	/*
	 * Top line of the screen
	 */
	{ "top.spinner",		CYAN,		NO_CHANGE,	BOLD	},
	{ "top.ports",			GREEN,		NO_CHANGE,	BOLD	},
	{ "top.page",			WHITE,		NO_CHANGE,	BOLD	},
	{ "top.copyright",		YELLOW,		NO_CHANGE,	OFF		},
	{ "top.poweredby",		BLUE,		NO_CHANGE,	BOLD	},

	/*
	 * Separator between displayed values and command history
	 */
	{ "sep.dash",			BLUE,		NO_CHANGE,	OFF		},
	{ "sep.text",			WHITE,		NO_CHANGE,	OFF		},

	/*
	 * Stats screen
	 */
	/* Port related */
	{ "stats.port.label",	BLUE,		NO_CHANGE,	BOLD	},
	{ "stats.port.flags",	BLUE,		NO_CHANGE,	BOLD	},
	{ "stats.port.status",	YELLOW,		NO_CHANGE,	BOLD	},

	/* Dynamic elements (updated every second) */
	{ "stats.dyn.label",	YELLOW,		NO_CHANGE,	OFF		},
	{ "stats.dyn.values",	YELLOW,		NO_CHANGE,	OFF		},

	/* Static elements (only update when explicitly set to different value) */
	{ "stats.stat.label",	MAGENTA,	NO_CHANGE,	OFF		},
	{ "stats.stat.values",	WHITE,		NO_CHANGE,	BOLD	},

	/* Total statistics */
	{ "stats.total.label",	RED,		NO_CHANGE,	BOLD	},

	/* Colon separating labels and values */
	{ "stats.colon",		BLUE,		NO_CHANGE,	BOLD	},

	/*
	 * Misc.
	 */
	{ "pktgen.prompt",		GREEN,		NO_CHANGE,	OFF		},
	{ NULL, 0, 0, 0 }
};


/* Initialize screen data structures */
void
pktgen_init_screen(int theme)
{
	pktgen.scrn = wr_scrn_init(MAX_SCRN_ROWS, MAX_SCRN_COLS, theme);
}


/* Print out the top line on the screen */
void
display_topline(const char * msg)
{
	wr_scrn_printf(1, 20, "%s", msg);
	pktgen_display_set_color("top.copyright");
	wr_scrn_puts("  %s", wr_copyright_msg());
	pktgen_display_set_color(NULL);
}


/* Print out the dashed line on the screen. */
void
display_dashline(int last_row)
{
	int i;

	wr_scrn_setw(last_row);
	last_row--;
	wr_scrn_pos(last_row, 1);
	pktgen_display_set_color("sep.dash");
	for(i=0; i<(__scrn->ncols-15); i++)
		wr_scrn_fprintf(0, 0, stdout, "-");
	pktgen_display_set_color("sep.text");
	wr_scrn_printf(last_row, 3, " Pktgen %s ", pktgen_version());
	pktgen_display_set_color("top.poweredby");
	wr_scrn_puts(" %s ", wr_powered_by());
	pktgen_display_set_color(NULL);
}

/* Set the display geometry */
void
pktgen_display_set_geometry(uint16_t rows, uint16_t cols)
{
	__scrn->nrows = rows;
	__scrn->ncols = cols;
}

/* Get the display geometry */
void
pktgen_display_get_geometry(uint16_t *rows, uint16_t *cols)
{
	if (rows != NULL)
		*rows = __scrn->nrows;

	if (cols != NULL)
		*cols = __scrn->ncols;
}


/* Look up the named color in the colormap */
static theme_color_map_t *
lookup_item(const char *elem)
{
	theme_color_map_t *result;

	if (elem == NULL)
		elem = "default";

	/* Look up colors and attributes for elem */
	for (result = theme_color_map; result->name != NULL; ++result) {
		if (strncasecmp(result->name, elem, MAX_COLOR_NAME_SIZE) == 0) {
			break;
		}
	}

	/* Report failure if element is not found */
	if (result->name == NULL)
		result = NULL;

	return result;
}


/* Changes the color to the color of the specified element */
void
pktgen_display_set_color(const char *elem) {
	theme_color_map_t *theme_color;

	if ( __scrn->theme == THEME_OFF )
		return;

	theme_color = lookup_item(elem);
	if (theme_color == NULL) {
		pktgen_log_error("Unknown color '%s'", elem);
		return;
	}

	wr_scrn_color(theme_color->fg_color, theme_color->bg_color, theme_color->attr);
}


/* String to use as prompt, with proper ANSI color codes */
void
__set_prompt(void)
{
	theme_color_map_t *def, *prompt;

	// Set default return value.
	snprintf(prompt_str, sizeof(prompt_str), "%s> ", PKTGEN_APP_NAME);

	if ( (__scrn->theme == THEME_ON) && !wr_scrn_is_paused() ) {
		// Look up the default and prompt values
		def    = lookup_item(NULL);
		prompt = lookup_item("pktgen.prompt");

		if ( (def == NULL) || (prompt == NULL) )
			pktgen_log_error("Prompt and/or default color undefined");
		else
			snprintf(prompt_str, sizeof(prompt_str), "\033[%d;%d;%dm%s>\033[%d;%d;%dm ",
					prompt->attr, 30 + prompt->fg_color, 40 + prompt->bg_color,
					PKTGEN_APP_NAME,
					def->attr,    30 + def->fg_color,    40 + def->bg_color);
	}

	cmdline_set_prompt(pktgen.cl, prompt_str);
}


static const char *
get_name_by_color(color_e color)
{
	int		i;

	for(i = 0; string_color_map[i].name; i++)
		if ( color == string_color_map[i].color )
			return string_color_map[i].name;
	return NULL;
}


static const char *
get_name_by_attr(attr_e attr)
{
	int		i;

	for(i = 0; string_attr_map[i].name; i++)
		if ( attr == string_attr_map[i].attr )
			return string_attr_map[i].name;
	return NULL;
}


static color_e
get_color_by_name(char * name)
{
	int		i;

	for(i = 0; string_color_map[i].name; i++)
		if ( strcmp(name, string_color_map[i].name) == 0 )
			return string_color_map[i].color;
	return UNKNOWN_COLOR;
}


static attr_e
get_attr_by_name(char * name)
{
	int		i;

	for(i = 0; string_attr_map[i].name; i++)
		if ( strcmp(name, string_attr_map[i].name) == 0 )
			return string_attr_map[i].attr;
	return UNKNOWN_ATTR;
}


void
pktgen_theme_show(void)
{
	int		i;

	printf("*** Theme Color Map Names (%s) ***\n", __scrn->theme ? "Enabled" : "Disabled");
	printf("   %-30s %-10s %-10s %s\n", "name", "FG Color", "BG Color", "Attribute");
	for(i=0; theme_color_map[i].name; i++) {
		printf("   %-32s %-10s %-10s %-6s",
			theme_color_map[i].name,
			get_name_by_color(theme_color_map[i].fg_color),
			get_name_by_color(theme_color_map[i].bg_color),
			get_name_by_attr(theme_color_map[i].attr));
		printf("     ");
		pktgen_display_set_color(theme_color_map[i].name);
		printf("%-s", theme_color_map[i].name);
		pktgen_display_set_color(NULL);
		printf("\n");
	}
}


void
pktgen_theme_state(const char * state)
{
	if ( parseState(state) == DISABLE_STATE )
		__scrn->theme = THEME_OFF;
	else
		__scrn->theme = THEME_ON;
	__set_prompt();
}

void
pktgen_set_theme_item( char * item, char * fg_color, char * bg_color, char * attr)
{
	theme_color_map_t *	elem;
	color_e	fg, bg;
	attr_e	at;

	elem = lookup_item(item);

	if ( elem == NULL ) {
		pktgen_log_error("Unknown item name (%s)\n", item);
		return;
	}

	fg = get_color_by_name(fg_color);
	bg = get_color_by_name(bg_color);
	at = get_attr_by_name(attr);

	if ( (fg == UNKNOWN_COLOR) || (bg == UNKNOWN_COLOR) || (at == UNKNOWN_ATTR) ) {
		pktgen_log_error("Unknown color or attribute (%s, %s, %s)\n", fg_color, bg_color, attr);
		return;
	}

	elem->fg_color	= fg;
	elem->bg_color	= bg;
	elem->attr		= at;
}

void
pktgen_theme_save(char * filename)
{
	FILE *	f;
	int		i;

	f = fopen(filename, "w+");
	if ( f == NULL ) {
		pktgen_log_error("Unable to open file %s\n", filename);
		return;
	}

	for(i = 0; theme_color_map[i].name; i++) {
		fprintf(f, "theme %s %s %s %s\n",
			theme_color_map[i].name,
			get_name_by_color(theme_color_map[i].fg_color),
			get_name_by_color(theme_color_map[i].bg_color),
			get_name_by_attr(theme_color_map[i].attr));
	}	
	fprintf(f, "cls\n");

	fclose(f);
}
