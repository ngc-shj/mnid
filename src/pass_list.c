/*
 * mnid
 *
 * Copyright(C) 2005 NOGUCHI Shoji
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "pass_list.h"
#include "pass.h"
#include "utils.h"

typedef struct _PassListWindow		PassListWindow;

enum col_titles {
	COL_DATE,
	COL_DATA,	/* PassInfo */
	N_COL_TITLES,
};

static struct _PassListWindow
{
	GtkWidget *dialog;
	GtkWidget *treeview;
	GtkTreeModel *model;

	GtkWidget *pass_entry;

	GSList *pass_list;

	gboolean is_finished;
} self;

static void pass_list_window_create		(void);
static void pass_list_window_create_list	(void);
static void pass_list_window_set_row		(PassInfo	*passinfo);

/* callback functions */
static gint pass_list_window_key_pressed	(GtkWidget	*widget,
						 GdkEventKey	*event,
						 gpointer	 data);
static void pass_list_window_response		(GtkDialog	*dialog,
						 gint		 reponse_id,
						 gpointer	 data);
static void pass_list_window_ok			(GtkWidget	*widget,
						 gpointer	 data);
static void pass_list_window_row_activated	(GtkTreeView	*treeview,
						 GtkTreePath	*path,
						 GtkTreeViewColumn
						 		*column,
						 gpointer	 data);
static void pass_list_window_cursor_changed	(GtkTreeView	*treeview,
						 gpointer	 data);

void pass_list_window_open(GSList *pass_list)
{
	if(!self.dialog)
		pass_list_window_create();

	self.is_finished = FALSE;
	self.pass_list = pass_list;
	pass_list_window_create_list();

	while(self.is_finished == FALSE){
		gtk_dialog_run(GTK_DIALOG(self.dialog));
	}
	gtk_widget_hide(self.dialog);
}

static void pass_list_window_create(void)
{
	GtkWidget *dialog;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *scrolledwin;
	GtkWidget *treeview;
	GtkWidget *pass_entry;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *model;
	gchar *buf;

	/* dialog */
	buf = g_strdup_printf("%s - %s", _("History"), FORMAL_NAME);
	dialog = gtk_dialog_new_with_buttons
			(buf,
			 NULL,
			 GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
			 GTK_STOCK_OK, GTK_RESPONSE_OK,
			 NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	gtk_widget_set_size_request(dialog, 400, 240);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), WIN_BORDER);
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_widget_realize(dialog);

	vbox = GTK_DIALOG(dialog)->vbox;
	gtk_box_set_spacing(GTK_BOX(vbox), VSPACING);

	/* treeview */
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	scrolledwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrolledwin);
	gtk_box_pack_start(GTK_BOX(hbox), scrolledwin, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
					    GTK_SHADOW_IN);

	/* Create the list model. */
	model = gtk_list_store_new(N_COL_TITLES,
				   G_TYPE_STRING,
				   G_TYPE_POINTER);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	g_object_unref(G_OBJECT(model));
	gtk_widget_show(treeview);
	gtk_container_add(GTK_CONTAINER(scrolledwin), treeview);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_selection_set_mode
		(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)),
		 GTK_SELECTION_BROWSE);

#define TREE_VIEW_APPEND_TEXT_COLUMN(__title, __pos, __width) \
{ \
	renderer = gtk_cell_renderer_text_new(); \
	column = gtk_tree_view_column_new_with_attributes(__title, \
							  renderer, "text", \
							  __pos, NULL); \
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column); \
	gtk_tree_view_column_set_sizing(column, \
					GTK_TREE_VIEW_COLUMN_FIXED); \
	gtk_tree_view_column_set_fixed_width(column, __width); \
	gtk_tree_view_column_set_resizable(column, TRUE); \
}

	TREE_VIEW_APPEND_TEXT_COLUMN(_("Update Date"), COL_DATE, 160);

#undef TREE_VIEW_APPEND_TEXT_COLUMN

	/* table */
	table = gtk_table_new(5, 2, FALSE);
	gtk_widget_show(table);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 8);

#define TABLE_SET_ATTACH_LABEL(name, left, right, top, bottom) \
{ \
	label = gtk_label_new(name); \
	gtk_widget_show(label); \
	gtk_table_attach(GTK_TABLE(table), label, \
			 left, right, top, bottom, \
			 GTK_FILL, 0, 0, 0); \
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT); \
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5); \
}

        /* pass */
	TABLE_SET_ATTACH_LABEL(_("Password"), 0, 1, 1, 2);
	pass_entry = gtk_entry_new();
	gtk_widget_show(pass_entry);
	gtk_table_attach(GTK_TABLE(table), pass_entry,
			 1, 2, 1, 2,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);
	gtk_editable_set_editable(GTK_EDITABLE(pass_entry), FALSE);

	/* signal connect */
	/* g_signal_connect(G_OBJECT(treeview), "row-activated", 
			 G_CALLBACK(pass_list_window_row_activated), NULL); */
	g_signal_connect(G_OBJECT(treeview), "cursor-changed", 
			 G_CALLBACK(pass_list_window_cursor_changed), NULL);
	g_signal_connect(G_OBJECT(dialog), "key_press_event",
			 G_CALLBACK(pass_list_window_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(pass_list_window_response), NULL);

	/* pointer */
	self.dialog	= dialog;
	self.treeview	= treeview;
	self.model	= GTK_TREE_MODEL(model);
	self.pass_entry	= pass_entry;
}

void pass_list_window_set_row(PassInfo *passinfo)
{
	GtkTreeIter iter;
	const gchar *text[N_COL_TITLES];
	struct tm *update;
	gchar buf[256];

	if(passinfo->update_date){
		update = localtime((time_t *)&passinfo->update_date);
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", update);
	}else
		strcpy(buf, "---------- --:--:--");
	text[COL_DATE] = buf;

	gtk_list_store_append(GTK_LIST_STORE(self.model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(self.model), &iter,
			   COL_DATE,	text[COL_DATE],
			   COL_DATA,	passinfo,
			   -1);
}

static void pass_list_window_create_list(void)
{
	GSList *cur;
	GtkTreeIter iter;

	gtk_list_store_clear(GTK_LIST_STORE(self.model));
	gtk_entry_set_text(GTK_ENTRY(self.pass_entry), "");

	for(cur = self.pass_list; cur; cur = cur->next)
		pass_list_window_set_row((PassInfo *)cur->data);

	if(gtk_tree_model_get_iter_first(self.model, &iter)){
		GtkTreePath *path;

		path = gtk_tree_model_get_path(self.model, &iter);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(self.treeview), path,
					 NULL, FALSE);
		gtk_tree_path_free(path);
	}
}

/* callback functions */
static gint pass_list_window_key_pressed(GtkWidget *widget, GdkEventKey *event,
				gpointer data)
{
	if(event && event->keyval == GDK_Escape){
		gtk_dialog_response(GTK_DIALOG(self.dialog),
				    GTK_RESPONSE_OK);
		return TRUE;
	}
	return FALSE;
}

static void pass_list_window_response(GtkDialog *dialog, gint response_id,
				    gpointer data)
{
	switch(response_id)
	{
	case GTK_RESPONSE_OK:
		pass_list_window_ok(NULL, data);
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		/* Hmm... argv dialog */
		gtk_dialog_response(GTK_DIALOG(self.dialog),
				    GTK_RESPONSE_CANCEL);
		break;
	case GTK_RESPONSE_NONE:
		break;
	default:
		break;
	}
}

static void pass_list_window_ok(GtkWidget *widget, gpointer data)
{
	self.is_finished = TRUE;
}

static void pass_list_window_row_activated(GtkTreeView *treeview,
						GtkTreePath *path,
						GtkTreeViewColumn *column,
						gpointer data)
{
	PassInfo *passinfo;
	GtkTreeIter iter;

	gtk_tree_model_get_iter(self.model, &iter, path);
	gtk_tree_model_get(self.model, &iter,
			   COL_DATA, &passinfo,
			   -1);

	gtk_entry_set_text(GTK_ENTRY(self.pass_entry), passinfo->pass);
}

static void pass_list_window_cursor_changed	(GtkTreeView	*treeview,
						 gpointer	 data)
{
	GtkTreePath *path;

	gtk_tree_view_get_cursor(GTK_TREE_VIEW(self.treeview), &path, NULL);
	if(!path){
		debug_print("nothing selected\n");
		return;
	}

	pass_list_window_row_activated(GTK_TREE_VIEW(self.treeview), path,
				       NULL, NULL);
	gtk_tree_path_free(path);
}
