/*-
 * Copyright (c) <2016>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Created 2016 by Abhinandan Gujjar S (abhinandan.gujjar@intel.com) */

#include "pktgen-gui.h"

/**************************************************************************//**
 *
 * check_entry - A routine to validate entry
 *
 * DESCRIPTION
 * verify that path is a valid node in tree
 *
 * RETURNS: TRUE/FALSE
 *
 * SEE ALSO:
 */

gboolean
check_entry(GtkTreeStore *tree, GtkTreePath *path)
{
	GtkTreeIter iter;

	return gtk_tree_model_get_iter(GTK_TREE_MODEL(tree), &iter, path);
}

/**************************************************************************//**
 *
 * add_entry - A routine to add entry
 *
 * DESCRIPTION
 * inserts a new node at path in tree
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

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
			while (!(gtk_tree_model_iter_n_children(GTK_TREE_MODEL(tree), NULL) == (index + 1)))
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

/**************************************************************************//**
 *
 * enable_stream_callback - A callback to enable/disable stream
 *
 * DESCRIPTION
 * Enable/Disable streams for a selected port
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_stream_callback(GtkCellRendererToggle *cell,
                       gchar     *path,
                       gpointer __attribute__((unused)) data)
{
	GtkTreeIter iter;
	gboolean active;
	port_info_t *info = NULL;
	pkt_seq_t *pkt = NULL;
	unsigned int pid = 0, seq_id = 0;

	GtkTreeSelection  *selection_chassis = gtk_tree_view_get_selection(
	                GTK_TREE_VIEW(chassis_view));

	gtk_tree_selection_selected_foreach(selection_chassis,
	                                    traffic_stream_get_pid,
	                                    (gpointer) & pid);

	GtkTreeModel  *model =
	        gtk_tree_view_get_model(GTK_TREE_VIEW(stream_view[pid]));

	g_object_get(G_OBJECT(cell), "active", &active, NULL);
	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
	                   TRAF_STR_SELECT, !active, -1);

	gtk_tree_model_get(model, &iter, TRAF_STR_NO, &seq_id, -1);

	info = &pktgen.info[pid];
	pkt  = &info->seq_pkt[seq_id];
	pkt->seq_enabled = !active;
}

/**************************************************************************//**
 *
 * digits_scale_callback - A callback to set digits
 *
 * DESCRIPTION
 * Round off and set the digits
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
digits_scale_callback(GtkAdjustment __attribute__((unused)) *adj)
{
	/* Set the number of decimal places to which adj->value is rounded */
	tx_rate = (gint)adj->value;
	gtk_scale_set_digits(GTK_SCALE(hscale), (gint)0);
}

/**************************************************************************//**
 *
 * apply_callback - A callback for applying changes on stream
 *
 * DESCRIPTION
 * Applying changes on stream
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
apply_callback(GtkWidget __attribute__(
                       (unused)) *w, gpointer __attribute__(
                       (unused)) data)
{
	guint flag = 1;
	GtkTreeSelection  *selection = gtk_tree_view_get_selection(
	                GTK_TREE_VIEW(chassis_view));

	gtk_tree_selection_selected_foreach(selection,
	                                    stream_apply,
	                                    (gpointer) & flag);
}

/**************************************************************************//**
 *
 * stream_apply - A routine for applying changes on stream
 *
 * DESCRIPTION
 * Applying changes on stream
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
stream_apply(GtkTreeModel  *model,
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

/**************************************************************************//**
 *
 * validate_ip_address - A routine for validating IP from user input
 *
 * DESCRIPTION
 * Validate user provided IP address
 *
 * RETURNS: 0=invalid or 1=valid
 *
 * SEE ALSO:
 */

int
validate_ip_address(char *st)
{
	int num, i, len;
	char *ch;

	/* counting number of quads present in a given IP address */
	int quadsCnt = 0;

	len = strlen(st);

	/*  Check if the string is valid */
	if (len < 7 || len > 15)
		return 0;

	ch = strtok(st, ".");

	while (ch != NULL) {
		quadsCnt++;

		num = 0;
		i = 0;

		/*  Get the current token and convert to an integer value */
		while (ch[i] != '\0') {
			num = num * 10;
			num = num + (ch[i] - '0');
			i++;
		}

		if (num < 0 || num > 255)
			/* Not a valid ip */
			return 0;

		if ( (quadsCnt == 1 && num == 0) || (quadsCnt == 4 && num == 0))
			/* Not a valid ip */
			return 0;

		ch = strtok(NULL, ".");
	}

	/*  Check the address string, should be n.n.n.n format */
	if (quadsCnt != 4)
		return 0;

	/*  Looks like a valid IP address */
	return 1;
}

/**************************************************************************//**
 *
 * traffic_stream_get_seq_id - A routine to get seq id for selected port
 *
 * DESCRIPTION
 * Get seq id for selected port
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
traffic_stream_get_seq_id(GtkTreeModel  *model,
                          GtkTreePath __attribute__((unused)) *path,
                          GtkTreeIter   *iter,
                          gpointer data)
{
	unsigned int *seq_id = (unsigned int *)data;

	gtk_tree_model_get(model, iter, TRAF_STR_NO, seq_id, -1);
}

/**************************************************************************//**
 *
 * traffic_stream_get_pid - A routine to get port id for selected port
 *
 * DESCRIPTION
 * Get port id for selected port
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
traffic_stream_get_pid(GtkTreeModel  *model,
                       GtkTreePath __attribute__((unused)) *path,
                       GtkTreeIter   *iter,
                       gpointer data)
{
	unsigned int *pid = (unsigned int *)data;
	gchar *name;

	gtk_tree_model_get(model, iter, COL_CHASSIS_PORTS, &name, -1);

	if (0 != g_strcmp0(name, "[127.0.0.1]")) {
		int offset = strlen(name);
		*pid = atoi((name + offset) - 1);
	}
}

/**************************************************************************//**
 *
 * hex_to_number - A routine to convert hex values into number
 *
 * DESCRIPTION
 * Convert hex to number
 *
 * RETURNS: integer
 *
 * SEE ALSO:
 */

int
hex_to_number(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

/**************************************************************************//**
 *
 * ascii_to_number - A routine to convert ascii values into number
 *
 * DESCRIPTION
 * Convert ascii to number
 *
 * RETURNS: integer
 *
 * SEE ALSO:
 */

int
ascii_to_number(const char *txt, unsigned int *addr, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		int a, b;
		a = hex_to_number(*txt++);
		if (a < 0)
			return -1;
		b = hex_to_number(*txt++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
	}
	return 0;
}
