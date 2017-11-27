/*-
 * Copyright (c) <2016>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Created 2016 by Abhinandan Gujjar S (abhinandan.gujjar@intel.com) */

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

extern pktgen_t pktgen;
extern cmdline_parse_ctx_t main_ctx[];

GtkTreeStore        *treestore_stats[RTE_MAX_ETHPORTS];
GtkTreeStore        *treestore_static[RTE_MAX_ETHPORTS];
GtkWidget           *view_static[RTE_MAX_ETHPORTS];
GtkTreeModel        *model_static[RTE_MAX_ETHPORTS];

GtkWidget           *view_stats[RTE_MAX_ETHPORTS];
GtkTreeModel        *model_stats[RTE_MAX_ETHPORTS];

GtkTextBuffer       *buffer;
GtkTextIter          buffer_iter;

GtkWidget           *stream_view[RTE_MAX_ETHPORTS];
GtkTreeStore        *traffic_stream[RTE_MAX_ETHPORTS];
GtkWidget           *stream_window;
GtkWidget           *hscale;
gint tx_rate;
GtkWidget           *notebook;
GtkScrolledWindow   *scroller;

GtkWidget           *chassis_view;


/* pktgen_port_stream data structure */
typedef struct pktgen_port_stream_t {
	gchar *path;
	gchar *stream_name;
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

/* Function prototypes */
GtkTreeModel *fill_port_info(unsigned int pid, gboolean is_static);
GtkWidget *chassis_tree_view(void);
GtkTreeModel *fill_chassis_info(void);
GtkWidget *create_chassis(void);
GtkWidget *console_box(const char *title);
GtkWidget *create_stats_treeview(gboolean is_static);
GtkTreeModel *stats_header_fill(gboolean is_static);
void pktgen_gui_main(int argc, char *argv[]);
void start_gui(void);
void close_gui(void);
int update_port_statistics(void *arg);
void update_port_static_info(unsigned int pid);
GtkWidget *show_statistics(void);
GtkWidget *show_static_conf(void);
gboolean check_entry(GtkTreeStore *tree, GtkTreePath *path);
void add_entry(GtkTreeStore *tree, GtkTreePath *path);
GtkWidget *stream_box(void);
void show_stream(void);
GtkWidget *pktgen_stream_box(void);
void edit_stream(void);
void set_stream_info(unsigned int pid, unsigned int seq_id);
void switch_stream_editor_page(GtkButton *, GtkNotebook *);
GtkWidget *fill_stream_info(proto_type type,
                            unsigned int pid,
                            unsigned int seq_id);
void add_proto_values_column(GtkTreeView  *treeview, GtkTreeModel *stream_model);
GtkTreeModel *create_stream_model(proto_type type,
                                  unsigned int pid,
                                  unsigned int seq_id);
void fill_proto_field_info(proto_type type,
                           unsigned int pid,
                           unsigned int seq_id);
int hex_to_number(char c);
int ascii_to_mac(const char *txt, unsigned int *addr);
int validate_ip_address(char *st);
int ascii_to_number(const char *txt, unsigned int *addr, int len);

void start_stop_traffic(GtkTreeModel  *model,
                        GtkTreePath   *path,
                        GtkTreeIter   *iter,
                        gpointer userdata);

void start_stop_capture(GtkTreeModel  *model,
                        GtkTreePath  *path,
                        GtkTreeIter   *iter,
                        gpointer userdata);

GtkWidget *port_tree_view(unsigned int port_id,
                          const char *title,
                          gboolean is_static);

void display_stream_editor(GtkWidget *stream_window,
                           unsigned int pid,
                           unsigned int seq_id);

void stream_apply(GtkTreeModel  *model,
                  GtkTreePath *path,
                  GtkTreeIter   *iter,
                  gpointer userdata);

void traffic_stream_get_pid(GtkTreeModel  *model,
                            GtkTreePath  *path,
                            GtkTreeIter   *iter,
                            gpointer data);
void traffic_stream_get_seq_id(GtkTreeModel  *model,
                               GtkTreePath  *path,
                               GtkTreeIter   *iter,
                               gpointer data);

/* Callback functions */
void console_callback(GtkWidget *widget, GtkWidget *entry);
void close_window_callback(GtkWidget *widget, gpointer window);
void apply_stream_callback(void);
void pktsize_enter_callback(GtkWidget *widget, gpointer *data);
void radio_options_callback(GtkRadioButton *b,  gpointer *user_data);
void apply_callback(GtkWidget *w, gpointer data);
void digits_scale_callback(GtkAdjustment *adj);
void start_taffic_callback(GtkWidget *w, gpointer data);
void stop_traffic_callback(GtkWidget *w, gpointer data);
void start_capture_callback(GtkWidget *w, gpointer data);
void stop_capture_callback(GtkWidget *w, gpointer data);
void about_dialog_callback(void);
void vlan_enable_callback(GtkWidget *widget, gpointer *data);

void enable_stream_callback(GtkCellRendererToggle *cell,
                            gchar  *path,
                            gpointer data);

void cell_edited_callback(GtkCellRendererText *cell,
                          const gchar         *path_string,
                          gchar         *new_text,
                          gpointer data);

void edit_stream_callback(GtkTreeModel  *model,
                          GtkTreePath  *path,
                          GtkTreeIter   *iter,
                          gpointer userdata);

void show_stream_callback(GtkTreeModel  *model,
                          GtkTreePath  *path,
                          GtkTreeIter   *iter,
                          gpointer userdata);

#endif
