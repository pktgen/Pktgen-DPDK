/*-
 * Copyright (c) <2016>, Intel Corporation
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
 * Copyright (c) <2016>, Intel. All rights reserved.
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
/* Created 2016 by Abhinandan Gujjar S (abhinandan.gujjar@intel.com) */

#ifndef _PKTGEN_GUI_H_
#define _PKTGEN_GUI_H_

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pktgen.h"
#include "pktgen-port-cfg.h"
#include "pktgen-cmds.h"
#include "lauxlib.h"

#define PKTGEN_GUI_MAX_STATS    22
#define PKTGEN_GUI_MAX_STATIC   13

#define NO_ETH_FIELDS            4
#define NO_IP_FIELDS            15
#define NO_UDP_FIELDS            4

GtkTreeStore        *treestore_stats[RTE_MAX_ETHPORTS];
GtkTreeStore        *treestore_static[RTE_MAX_ETHPORTS];
GtkWidget           *view_static[RTE_MAX_ETHPORTS];
GtkTreeModel        *model_static[RTE_MAX_ETHPORTS];

GtkWidget           *view_chassis;
GtkWidget           *view_stats[RTE_MAX_ETHPORTS];
GtkTreeModel        *model_stats[RTE_MAX_ETHPORTS];

GtkWidget           *traffic_start_button;
GtkWidget           *traffic_stop_button;
GtkWidget           *capture_start_button;
GtkWidget           *capture_stop_button;
GtkWidget           *about_button;

GtkTextBuffer       *buffer;
GtkTextIter buffer_iter;

GtkTreeView         *stream_view;
GtkTreeStore        *traffic_stream;
GtkWidget           *stream_window;
GtkWidget           *hscale;
gint tx_rate;
GtkWidget           *notebook;

GtkWidget           *chassis_view;

typedef struct highlight_stats_s {
	uint64_t __64;		/**< Number of 64 byte packets */
	uint64_t __65_127;	/**< Number of 65-127 byte packets */
	uint64_t __128_255;	/**< Number of 128-255 byte packets */
	uint64_t __256_511;	/**< Number of 256-511 byte packets */
	uint64_t __512_1023;	/**< Number of 512-1023 byte packets */
	uint64_t __1024_1518;	/**< Number of 1024-1518 byte packets */
	uint64_t __broadcast;	/**< Number of broadcast packets */
	uint64_t __multicast;	/**< Number of multicast packets */
	uint64_t __ierrors;	/**< Number of Rx packets */
	uint64_t __oerrors;	/**< Number of Tx packets */
} highlight_stats_t;

extern pktgen_t pktgen;
extern cmdline_parse_ctx_t main_ctx[];

/* pktgen_port_stream data structure */
typedef struct pktgen_port_stream_t {
	const gchar *path;
	const gchar *stream_name;
	gboolean stream_select;
	gint stream_no;
} pktgen_port_stream;

/* enum for chassis treeview */
enum {
	COL_CHASSIS_PORTS = 0,
	NUM_COLS
};

/* column identifiers for stream editor tree view */
enum {
	TRAF_STR_NAME,
	TRAF_STR_SELECT,
	TRAF_STR_NO,
	NUM_TRAF_STR_COLS
};

/* column identifiers for protocol fields in tree view */
enum {
	SL_COL,
	SELECT_COL,
	NUM_PROTO_COLS
};

/* Enum for protocol stack */
typedef enum {
	TYPE_ETH,
	TYPE_VLAN,
	TYPE_IPv4,
	TYPE_UDP,
	TYPE_TCP,
	TYPE_INVALID
} proto_type;

/* Enum for protocol treeview */
enum {
	COLUMN_NAME,
	COLUMN_VALUE,
	MAX_COLUMNS
};

/* Display protocol values on treeview [name:value] format */
typedef struct {
	gchar *name;
	gchar *value;
}
protocol;

GtkWidget *port_tree_view(unsigned int port_id,
                          const char *title,
                          gboolean is_static);
GtkTreeModel *fill_port_info(unsigned int pid, gboolean is_static);

GtkWidget *chassis_tree_view(void);
GtkTreeModel *fill_chassis_info(void);

GtkWidget *pktgen_button_box(char *title, gint layout);
GtkWidget *create_chassis(void);
GtkWidget *pktgen_console_box(const char *title);

GtkWidget *pktgen_stats_header_tree_view(gboolean is_static);
GtkTreeModel *pktgen_stats_header_fill(gboolean is_static);

void pktgen_gui_main(int argc, char *argv[]);
void pktgen_start_gui(void);
void pktgen_gui_close(void);
int update_ports_stat(void *arg);
void update_ports_static_stat(unsigned int pid);
GtkWidget *pktgen_show_statistic_data(void);
GtkWidget *pktgen_show_static_conf(void);
GtkWidget *pktgen_show_total_data(void /*GtkWidget *frame_horz_stats, GtkWidget *scrolled_window*/);

void start_stop_traffic(GtkTreeModel  *model,
                        GtkTreePath   *path,
                        GtkTreeIter   *iter,
                        gpointer userdata);
void start_stop_capture(GtkTreeModel  *model,
                        GtkTreePath  *path,
                        GtkTreeIter   *iter,
                        gpointer userdata);

void pktgen_display_stream_editor(GtkWidget *stream_window, unsigned int pid);
GtkWidget *pktgen_stream_box(void);
void pktgen_edit_stream(void);
void pktgen_apply_traffic(GtkTreeModel  *model,
                          GtkTreePath *path,
                          GtkTreeIter   *iter,
                          gpointer userdata);

gboolean check_entry(GtkTreeStore *tree, GtkTreePath *path);
void add_entry(GtkTreeStore *tree, GtkTreePath *path);
void about_dialog(void);

GtkWidget *pktgen_stream_box(void);

void pktgen_set_stream_info(unsigned int pid);
void switch_stream_editor_page(GtkButton *, GtkNotebook *);
void pktgen_display_stream_editor(GtkWidget *window, unsigned int pid);
void pktgen_conf_traffic_stream(GtkTreeModel  *model,
                                GtkTreePath  *path,
                                GtkTreeIter   *iter,
                                gpointer userdata);
GtkWidget *pktgen_fill_stream_info(proto_type type, unsigned int pid);
void add_proto_values_column(GtkTreeView  *treeview, GtkTreeModel *stream_model);
GtkTreeModel *create_stream_model(proto_type type, unsigned int pid);
void fill_proto_field_info(proto_type type, unsigned int pid);

int hex_to_number(char c);
int ascii_to_mac(const char *txt, unsigned int *addr);
int validate_ip_address(char *st);

/* Callback functions */
void console_callback(GtkWidget *widget, GtkWidget *entry);
void close_window_callback(GtkWidget *widget, gpointer window);
void pktgen_port_stream_apply_callback(void);
void vlan_enable_callback(GtkWidget *widget, gpointer *data);
void cell_edited_callback(GtkCellRendererText *cell,
                          const gchar         *path_string,
                          gchar         *new_text,
                          gpointer data);

void traffic_stream_callback(GtkTreeModel  * model,
                             GtkTreePath __attribute__((unused)) * path,
                             GtkTreeIter   * iter,
                             gpointer userdata);

void traffic_apply_callback(GtkWidget *w, gpointer data);
void digits_scale_callback(GtkAdjustment *adj);
void traffic_start_callback(GtkWidget *w, gpointer data);
void traffic_stop_callback(GtkWidget *w, gpointer data);
void capture_start_callback(GtkWidget *w, gpointer data);
void capture_stop_callback(GtkWidget *w, gpointer data);
void about_callback(GtkWidget *w, gpointer data);

#endif
