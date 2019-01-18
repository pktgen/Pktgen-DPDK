/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <sys/stat.h>
#include "rte_lua.h"

#include "pktgen-display.h"
#include "pktgen-cmds.h"

#define MAX_COLOR_NAME_SIZE     64
#define MAX_PROMPT_STRING_SIZE  64

static char prompt_str[MAX_PROMPT_STRING_SIZE] = { 0 };

/* String to color value mapping */
typedef struct string_color_map_s {
	const char    *name;	/**< Color name */
	scrn_color_e color;		/**< Color value for scrn_{fg,bg}color() */
} string_color_map_t;

string_color_map_t string_color_map[] = {
	{ "black",      SCRN_BLACK       },
	{ "black",      SCRN_DEFAULT_FG  },
	{ "red",        SCRN_RED         },
	{ "green",      SCRN_GREEN       },
	{ "yellow",     SCRN_YELLOW      },
	{ "blue",       SCRN_BLUE        },
	{ "magenta",    SCRN_MAGENTA     },
	{ "cyan",       SCRN_CYAN        },
	{ "white",      SCRN_WHITE       },
	{ "white",      SCRN_DEFAULT_BG  },
	{ "default",    SCRN_WHITE       },	/* alias */

	{ "none",       SCRN_NO_CHANGE   },
	{ "default_fg", SCRN_NO_CHANGE   },
	{ "default_bg", SCRN_NO_CHANGE   },
	{ NULL, 0 }
};

/* String to attribute mapping */
typedef struct string_attr_map_s {
	const char    *name;	/**< Attribute name */
	scrn_attr_e attr;		/**< Attribute value for scrn_{fg,bg}color_attr() */
} string_attr_map_t;

string_attr_map_t string_attr_map[] = {
	{ "off",        SCRN_OFF         },
	{ "default",    SCRN_OFF         },	/* alias */
	{ "bold",       SCRN_BOLD        },
	{ "bright",     SCRN_BOLD        },	/* alias */
	{ "underscore", SCRN_UNDERSCORE  },
	{ "underline",  SCRN_UNDERSCORE  },	/* alias */
	{ "blink",      SCRN_BLINK       },
	{ "reverse",    SCRN_REVERSE     },
	{ "concealed",  SCRN_CONCEALED   },
	{ NULL, 0 }
};

/* Element to color mapping */
typedef struct theme_color_map_s {
	const char        *name;/**< Display element name */
	scrn_color_e fg_color;
	scrn_color_e bg_color;
	scrn_attr_e attr;
} theme_color_map_t;

theme_color_map_t theme_color_map[] = {
/*	{ "element name",		FG_COLOR,	BG_COLOR,	ATTR	} */
	{ "default",            SCRN_DEFAULT_FG, SCRN_DEFAULT_BG, SCRN_OFF     },

	/*
	 * Top line of the screen
	 */
	{ "top.spinner",        SCRN_CYAN,       SCRN_NO_CHANGE,  SCRN_BOLD    },
	{ "top.ports",          SCRN_GREEN,      SCRN_NO_CHANGE,  SCRN_BOLD    },
	{ "top.page",           SCRN_WHITE,      SCRN_NO_CHANGE,  SCRN_BOLD    },
	{ "top.copyright",      SCRN_YELLOW,     SCRN_NO_CHANGE,  SCRN_OFF     },
	{ "top.poweredby",      SCRN_BLUE,       SCRN_NO_CHANGE,  SCRN_BOLD    },

	/*
	 * Separator between displayed values and command history
	 */
	{ "sep.dash",           SCRN_BLUE,       SCRN_NO_CHANGE,  SCRN_OFF     },
	{ "sep.text",           SCRN_WHITE,      SCRN_NO_CHANGE,  SCRN_OFF     },

	/*
	 * Stats screen
	 */
	/* Port related */
	{ "stats.port.label",   SCRN_BLUE,       SCRN_NO_CHANGE,  SCRN_BOLD    },
	{ "stats.port.flags",   SCRN_BLUE,       SCRN_NO_CHANGE,  SCRN_BOLD    },
	{ "stats.port.status",  SCRN_YELLOW,     SCRN_NO_CHANGE,  SCRN_BOLD    },
	{ "stats.port.data",    SCRN_WHITE,      SCRN_NO_CHANGE,  SCRN_OFF     },

	/* Dynamic elements (updated every second) */
	{ "stats.dyn.label",    SCRN_YELLOW,     SCRN_NO_CHANGE,  SCRN_OFF     },
	{ "stats.dyn.values",   SCRN_YELLOW,     SCRN_NO_CHANGE,  SCRN_OFF     },

	/* Static elements (only update when explicitly set to different value) */
	{ "stats.stat.label",   SCRN_MAGENTA,    SCRN_NO_CHANGE,  SCRN_OFF     },
	{ "stats.stat.values",  SCRN_WHITE,      SCRN_NO_CHANGE,  SCRN_BOLD    },

	/* Total statistics */
	{ "stats.total.label",  SCRN_RED,        SCRN_NO_CHANGE,  SCRN_BOLD    },
	{ "stats.total.data",   SCRN_RED,        SCRN_NO_CHANGE,  SCRN_BOLD    },

	/* Colon separating labels and values */
	{ "stats.colon",        SCRN_BLUE,       SCRN_NO_CHANGE,  SCRN_BOLD    },

	/* Highlight some static values */
	{ "stats.rate.count",   SCRN_MAGENTA,    SCRN_NO_CHANGE,  SCRN_BOLD    },
	{ "stats.bdf",          SCRN_BLUE,       SCRN_NO_CHANGE,  SCRN_BOLD    },
	{ "stats.mac",          SCRN_GREEN,      SCRN_NO_CHANGE,  SCRN_BOLD    },
	{ "stats.ip",           SCRN_CYAN,       SCRN_NO_CHANGE,  SCRN_BOLD    },

	/*
	 * Misc.
	 */
	{ "pktgen.prompt",      SCRN_GREEN,      SCRN_NO_CHANGE,  SCRN_OFF     },
	{ NULL, 0, 0, 0 }
};

/* Initialize screen data structures */
/* Print out the top line on the screen */
void
display_topline(const char *msg)
{
	pktgen_display_set_color("top.page");
	scrn_printf(1, 20, "%s", msg);
	pktgen_display_set_color("top.copyright");
	scrn_puts("  %s", copyright_msg_short());
	pktgen_display_set_color(NULL);
}

/* Print out the dashed line on the screen. */
void
display_dashline(int last_row)
{
	int i;

	scrn_setw(last_row);
	last_row--;
	scrn_pos(last_row, 1);
	pktgen_display_set_color("sep.dash");
	for (i = 0; i < 79; i++)
		scrn_fprintf(0, 0, stdout, "-");
	pktgen_display_set_color("sep.text");
	scrn_printf(last_row, 3, " Pktgen %s ", pktgen_version());
	pktgen_display_set_color("top.poweredby");
	scrn_puts(" %s ", powered_by());
	scrn_puts(" (pid:%d) ", getpid());
	pktgen_display_set_color(NULL);
}

/* Set the display geometry */
void
pktgen_display_set_geometry(uint16_t rows, uint16_t cols)
{
	if (!this_scrn)
		return;
	this_scrn->nrows = rows;
	this_scrn->ncols = cols;
}

/* Get the display geometry */
void
pktgen_display_get_geometry(uint16_t *rows, uint16_t *cols)
{
	if (!this_scrn)
		return;

	if (rows != NULL)
		*rows = this_scrn->nrows;

	if (cols != NULL)
		*cols = this_scrn->ncols;
}

/* Look up the named color in the colormap */
static theme_color_map_t *
lookup_item(const char *elem)
{
	theme_color_map_t *result;

	if (elem == NULL)
		elem = "default";

	/* Look up colors and attributes for elem */
	for (result = theme_color_map; result->name != NULL; ++result)
		if (strncasecmp(result->name, elem, MAX_COLOR_NAME_SIZE) == 0)
			break;

	/* Report failure if element is not found */
	if (result->name == NULL)
		result = NULL;

	return result;
}

/* Changes the color to the color of the specified element */
void
pktgen_display_set_color(const char *elem) {
	theme_color_map_t *theme_color;

	if (!this_scrn || this_scrn->theme == SCRN_THEME_OFF)
		return;

	theme_color = lookup_item(elem);
	if (theme_color == NULL) {
		pktgen_log_error("Unknown color '%s'", elem);
		return;
	}

	scrn_color(theme_color->fg_color,
	              theme_color->bg_color,
	              theme_color->attr);
}

/* String to use as prompt, with proper ANSI color codes */
void
__set_prompt(void)
{
	theme_color_map_t *def, *prompt;

	if (!this_scrn)
		return;

	/* Set default return value. */
	snprintf(prompt_str, sizeof(prompt_str), "%s> ", PKTGEN_APP_NAME);

	if ( (this_scrn->theme == SCRN_THEME_ON) && !scrn_is_paused() ) {
		/* Look up the default and prompt values */
		def    = lookup_item(NULL);
		prompt = lookup_item("pktgen.prompt");

		if ( (def == NULL) || (prompt == NULL) )
			pktgen_log_error(
				"Prompt and/or default color undefined");

		else
			snprintf(prompt_str,
				 sizeof(prompt_str),
				 "\033[%d;%d;%dm%s>\033[%d;%d;%dm ",
				 prompt->attr,
				 30 + prompt->fg_color,
				 40 + prompt->bg_color,
				 PKTGEN_APP_NAME,
				 def->attr,
				 30 + def->fg_color,
				 40 + def->bg_color);
	}
}

static const char *
get_name_by_color(scrn_color_e color)
{
	int i;

	for (i = 0; string_color_map[i].name; i++)
		if (color == string_color_map[i].color)
			return string_color_map[i].name;
	return NULL;
}

static const char *
get_name_by_attr(scrn_attr_e attr)
{
	int i;

	for (i = 0; string_attr_map[i].name; i++)
		if (attr == string_attr_map[i].attr)
			return string_attr_map[i].name;
	return NULL;
}

static scrn_color_e
get_color_by_name(char *name)
{
	int i;

	for (i = 0; string_color_map[i].name; i++)
		if (strcmp(name, string_color_map[i].name) == 0)
			return string_color_map[i].color;
	return SCRN_UNKNOWN_COLOR;
}

static scrn_attr_e
get_attr_by_name(char *name)
{
	int i;

	for (i = 0; string_attr_map[i].name; i++)
		if (strcmp(name, string_attr_map[i].name) == 0)
			return string_attr_map[i].attr;
	return SCRN_UNKNOWN_ATTR;
}

void
pktgen_theme_show(void)
{
	int i;

	if (!this_scrn)
		return;

	printf("*** Theme Color Map Names (%s) ***\n",
	       this_scrn->theme ? "Enabled" : "Disabled");
	printf("   %-30s %-10s %-10s %s\n",
	       "name",
	       "FG Color",
	       "BG Color",
	       "Attribute");
	for (i = 0; theme_color_map[i].name; i++) {
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
pktgen_theme_state(const char *state)
{
	if (!this_scrn)
		return;
	if (estate(state) == DISABLE_STATE)
		this_scrn->theme = SCRN_THEME_OFF;
	else
		this_scrn->theme = SCRN_THEME_ON;
	__set_prompt();
}

void
pktgen_set_theme_item(char *item, char *fg_color, char *bg_color, char *attr)
{
	theme_color_map_t *elem;
	scrn_color_e fg, bg;
	scrn_attr_e at;

	elem = lookup_item(item);

	if (elem == NULL) {
		pktgen_log_error("Unknown item name (%s)\n", item);
		return;
	}

	fg = get_color_by_name(fg_color);
	bg = get_color_by_name(bg_color);
	at = get_attr_by_name(attr);

	if ( (fg == SCRN_UNKNOWN_COLOR) || (bg == SCRN_UNKNOWN_COLOR) ||
	     (at == SCRN_UNKNOWN_ATTR) ) {
		pktgen_log_error("Unknown color or attribute (%s, %s, %s)\n",
				 fg_color,
				 bg_color,
				 attr);
		return;
	}

	elem->fg_color  = fg;
	elem->bg_color  = bg;
	elem->attr      = at;
}

void
pktgen_theme_save(char *filename)
{
	FILE *f;
	int i;

	f = fopen(filename, "w+");
	if (f == NULL) {
		pktgen_log_error("Unable to open file %s\n", filename);
		return;
	}

	for (i = 0; theme_color_map[i].name; i++)
		fprintf(f, "theme %s %s %s %s\n",
			theme_color_map[i].name,
			get_name_by_color(theme_color_map[i].fg_color),
			get_name_by_color(theme_color_map[i].bg_color),
			get_name_by_attr(theme_color_map[i].attr));
	fprintf(f, "cls\n");

	fchmod(fileno(f), 0666);
	fclose(f);
}
