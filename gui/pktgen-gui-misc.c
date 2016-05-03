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

#include "pktgen-gui.h"

extern GtkWidget *window;

/* pktgen_port_stream data. TBD: To be enhanced to support multiple streams per port */
pktgen_port_stream str_db[] = {
	{ "0", "IPv4",  TRUE, 1 },
	{ NULL, NULL,  FALSE, -1}
};

void
pktgen_edit_stream(void)
{
	uint no_rows;

	stream_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(stream_window), "Edit Streams");
	gtk_window_set_default_size(GTK_WINDOW(stream_window), 400, 400);
	g_signal_connect(stream_window, "destroy", G_CALLBACK(
	                         gtk_main_quit), NULL);

	gtk_container_set_border_width(GTK_CONTAINER(stream_window), 10);

	{
		GtkTreeSelection  *selection = gtk_tree_view_get_selection(
		                GTK_TREE_VIEW(chassis_view));

		no_rows = gtk_tree_selection_count_selected_rows(selection);

		if (no_rows == 0) {
			GtkWidget *dialog;
			dialog = gtk_message_dialog_new(GTK_WINDOW(
			                                        window),
			                                GTK_DIALOG_DESTROY_WITH_PARENT,
			                                GTK_MESSAGE_ERROR,
			                                GTK_BUTTONS_OK,
			                                "Please select a port to edit traffic stream");
			gtk_window_set_title(GTK_WINDOW(dialog), "Pktgen");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else if (no_rows > 1) {
			GtkWidget *dialog;
			dialog = gtk_message_dialog_new(GTK_WINDOW(
			                                        window),
			                                GTK_DIALOG_DESTROY_WITH_PARENT,
			                                GTK_MESSAGE_ERROR,
			                                GTK_BUTTONS_OK,
			                                "Traffic stream editing for multiple ports cannot be done simultaneously!! \nPlease select a single port");
			gtk_window_set_title(GTK_WINDOW(dialog), "Pktgen");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else
			gtk_tree_selection_selected_foreach(
			        selection,
			        traffic_stream_callback,
			        (gpointer)
			        stream_window);
	}
}

void
traffic_stream_callback(GtkTreeModel  *model,
                        GtkTreePath __attribute__((unused)) *path,
                        GtkTreeIter   *iter,
                        gpointer userdata)
{
	gchar *name;
	unsigned int pid;
	GtkWidget *stream_window = (GtkWidget *)userdata;

	gtk_tree_model_get(model, iter, COL_CHASSIS_PORTS, &name, -1);

	if (0 != g_strcmp0(name, "[127.0.0.1]")) {
		int offset = strlen(name);
		pid = atoi((name + offset) - 1);
		pktgen_display_stream_editor(stream_window, pid);
	}
}

/* verify that path is a valid node in tree */
gboolean
check_entry(GtkTreeStore *tree, GtkTreePath *path)
{
	GtkTreeIter iter;

	return gtk_tree_model_get_iter(GTK_TREE_MODEL(tree), &iter, path);
}

/* inserts a new node at path in tree */
void
add_entry(GtkTreeStore *tree, GtkTreePath *path)
{
	gint depth;
	gint *indices;
	gint index;
	GtkTreeIter iter;

	/* determine depth and last index of path */
	depth = gtk_tree_path_get_depth(path);
	indices = gtk_tree_path_get_indices(path);
	index = indices[depth - 1];

	if (!check_entry(tree, path)) {
		if (depth == 1)	/* if this is a child of the root node, use NULL instead of iter */
			while (!(gtk_tree_model_iter_n_children(GTK_TREE_MODEL(
			                                                tree),
			                                        NULL) ==
			         (index + 1)))
				gtk_tree_store_append(tree, &iter, NULL);
		else {
			GtkTreePath *parent_path;
			GtkTreeIter parent;

			/* determine parent node, creating parent if it does not exist */
			parent_path = gtk_tree_path_copy(path);
			gtk_tree_path_up(parent_path);
			if (!check_entry(tree, parent_path))
				add_entry(tree, parent_path);
			/* append new nodes up to index-th child of parent */
			gtk_tree_model_get_iter(GTK_TREE_MODEL(tree), &parent,
			                        parent_path);
			while (!(gtk_tree_model_iter_n_children(
			                 GTK_TREE_MODEL(tree),
			                 &parent) == (index + 1)))
				gtk_tree_store_append(tree, &iter, &parent);

			gtk_tree_path_free(parent_path);
		}
	}
}

static void
cb_toggled(GtkCellRendererToggle *cell,
           gchar                 *path,
           GtkTreeModel          *model)
{
	GtkTreeIter iter;
	gboolean active;

	g_object_get(G_OBJECT(cell), "active", &active, NULL);

	gtk_tree_model_get_iter_from_string(model, &iter, path);

	gtk_tree_store_set(traffic_stream, &iter,
	                   TRAF_STR_SELECT, TRUE /*! active*/, -1);	/* Set to active by default. TBD: change it as per user input when enhanced for multiple streams */
}

void
digits_scale_callback(GtkAdjustment __attribute__((unused)) *adj)
{
	/* Set the number of decimal places to which adj->value is rounded */
	tx_rate = (gint)adj->value;
	gtk_scale_set_digits(GTK_SCALE(hscale), (gint)0);
}

void
traffic_apply_callback(GtkWidget __attribute__(
                               (unused)) *w, gpointer __attribute__(
                               (unused)) data)
{
	guint flag = 1;
	GtkTreeSelection  *selection = gtk_tree_view_get_selection(
	                GTK_TREE_VIEW(chassis_view));

	gtk_tree_selection_selected_foreach(selection,
	                                    pktgen_apply_traffic,
	                                    (gpointer) & flag);
}

void
pktgen_apply_traffic(GtkTreeModel  *model,
                     GtkTreePath __attribute__((unused)) *path,
                     GtkTreeIter   *iter,
                     gpointer userdata)
{
	gchar *name;
	port_info_t *info = NULL;
	unsigned int pid;
	guint *flag = (guint *)userdata;

	gtk_tree_model_get(model, iter, COL_CHASSIS_PORTS, &name, -1);

	if (0 != g_strcmp0(name, "[127.0.0.1]")) {
		int offset = strlen(name);
		pid = atoi((name + offset) - 1);
		info = &pktgen.info[pid];
		if ((info != NULL) && (*flag == 1))
			pktgen_set_tx_rate(info, tx_rate);
	} else if (*flag == 1)
		forall_ports(pktgen_set_tx_rate(info, tx_rate));
}

GtkWidget *
pktgen_stream_box(void)
{
	pktgen_port_stream *port_traffic;
	GtkTreePath *path;
	GtkScrolledWindow *scroller;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkObject *tx_rate_range;
	GtkWidget *apply_button;

	GtkCellRenderer *num_renderer, *bold_renderer, *check_renderer;
	GtkTreeViewColumn *sl_no_col, *stream_name_col, *select_col;

	frame = gtk_frame_new("Configuration");
	gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
	gtk_widget_set_size_request(frame, 220, 200);

	apply_button = gtk_button_new_with_label("Apply");
	gtk_widget_set_tooltip_text(GTK_WIDGET(apply_button), "Apply");

	gtk_signal_connect(GTK_OBJECT(apply_button),
	                   "clicked",
	                   GTK_SIGNAL_FUNC(traffic_apply_callback),
	                   NULL);

	tx_rate_range = gtk_adjustment_new(1, 1.0, 101.0, 1.0, 1.0, 1.0);
	g_signal_connect(tx_rate_range, "value_changed",
	                 G_CALLBACK(digits_scale_callback), NULL);

	vbox = gtk_vbox_new(FALSE, 10);
	hbox = gtk_hbox_new(FALSE, 10);

	/* Reuse the same adjustment */
	hscale = gtk_hscale_new(GTK_ADJUSTMENT(tx_rate_range));
	gtk_scale_set_digits(GTK_SCALE(hscale), (gint)0);
	gtk_range_set_update_policy(GTK_RANGE(hscale),
	                            GTK_UPDATE_CONTINUOUS);
	gtk_scale_set_digits(GTK_SCALE(hscale), 1);
	gtk_scale_set_value_pos(GTK_SCALE(hscale), GTK_POS_TOP);
	gtk_scale_set_draw_value(GTK_SCALE(hscale), TRUE);

	gtk_box_pack_start(GTK_BOX(hbox), hscale, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), apply_button, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

	/* create tree model */
	traffic_stream = gtk_tree_store_new(NUM_TRAF_STR_COLS,
	                                    G_TYPE_STRING,
	                                    G_TYPE_BOOLEAN,
	                                    G_TYPE_INT);

	port_traffic = str_db;
	while (port_traffic->path != NULL) {
		GtkTreeIter iter;

		path = gtk_tree_path_new_from_string(port_traffic->path);
		add_entry(traffic_stream, path);
		gtk_tree_model_get_iter(GTK_TREE_MODEL(
		                                traffic_stream), &iter, path);
		gtk_tree_path_free(path);
		gtk_tree_store_set(traffic_stream, &iter,
		                   TRAF_STR_NAME, port_traffic->stream_name,
		                   TRAF_STR_SELECT, port_traffic->stream_select,
		                   TRAF_STR_NO, port_traffic->stream_no,
		                   -1);
		port_traffic++;
	}

	/* create a right-justified renderer for nums */
	num_renderer = gtk_cell_renderer_text_new();
	g_object_set(num_renderer, "xalign", 0.1, NULL);

	/* a renderer for text in boldface (for last name column) */
	bold_renderer = gtk_cell_renderer_text_new();
	g_object_set(bold_renderer, "weight", 500, NULL);

	/* a check box renderer */
	check_renderer = gtk_cell_renderer_toggle_new();

	/* create view columns */
	sl_no_col = gtk_tree_view_column_new_with_attributes(
	                "No.", num_renderer,
	                "text", TRAF_STR_NO,
	                NULL);

	stream_name_col = gtk_tree_view_column_new_with_attributes(
	                "Stream Name", bold_renderer,
	                "text", TRAF_STR_NAME,
	                NULL);

	g_signal_connect(G_OBJECT(check_renderer), "toggled",
	                 G_CALLBACK(cb_toggled), GTK_TREE_MODEL(traffic_stream) );

	select_col = gtk_tree_view_column_new_with_attributes(
	                "Select", check_renderer,
	                "active", TRAF_STR_SELECT,
	                NULL);

	/* create overall view */
	stream_view = g_object_new(GTK_TYPE_TREE_VIEW,
	                           "model", traffic_stream,
	                           "rules-hint", TRUE,
	                           "enable-search", TRUE,
	                           "search-column", TRAF_STR_NAME,
	                           NULL);

	g_signal_connect(stream_view,
	                 "row-activated",
	                 (GCallback)pktgen_edit_stream,
	                 NULL);

	gtk_tree_view_append_column(stream_view, sl_no_col);
	gtk_tree_view_append_column(stream_view, stream_name_col);
	gtk_tree_view_append_column(stream_view, select_col);

	scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller),
	                               GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_container_add(GTK_CONTAINER(scroller), GTK_WIDGET(stream_view));
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(scroller), TRUE, TRUE, 5);
	gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(vbox));
	return GTK_WIDGET(frame);
}
