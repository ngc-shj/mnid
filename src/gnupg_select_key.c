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

/*
 * We have copied a part of code from the following other programs.
 *
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 2001 Werner Koch (dd9jn)
 *   o select-keys.c
 *
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if USE_GPGME

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gpgme.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "gnupg_select_key.h"
#include "utils.h"

typedef struct _GnupgSelectKeyWindow		GnupgSelectKeyWindow;

enum col_titles {
	COL_ALGO,
	COL_KEYID,
	COL_NAME,
	COL_EMAIL,
	COL_VALIDITY,
	COL_POINTER,
	N_COL_TITLES,
};

static struct _GnupgSelectKeyWindow
{
	GtkWidget *dialog;
	GtkWidget *treeview;
	GtkTreeModel *model;

	const gchar *cur_keyid;
	gchar *new_keyid;

	gboolean is_finished;
} self;

static void gnupg_window_create		(void);
static void gnupg_window_create_keylist	(void);
static GtkTreePath *gnupg_window_set_row	(gpgme_key_t	 key);
static void gnupg_window_destroy_key		(gpointer	 data);

/* callback functions */
static gint gnupg_window_key_pressed		(GtkWidget	*widget,
						 GdkEventKey	*event,
						 gpointer	 data);
static void gnupg_window_response		(GtkDialog	*dialog,
						 gint		 reponse_id,
						 gpointer	 data);
static void gnupg_window_destroy		(GtkWidget	*widget,
						 gpointer	 data);
static void gnupg_window_ok			(GtkWidget	*widget,
						 gpointer	 data);
static void gnupg_window_cancel			(GtkWidget	*widget,
	    				 	 gpointer	 data);
static void gnupg_window_keylist_activated	(GtkTreeView	*treeview,
						 GtkTreePath	*path,
						 GtkTreeViewColumn
						 		*column,
						 gpointer	 data);

const gchar *gnupg_select_key_open(const gchar *keyid)
{
	if(!self.dialog)
		gnupg_window_create();

	self.cur_keyid = keyid;
	gnupg_window_create_keylist();

	self.is_finished = FALSE;
	while(self.is_finished == FALSE){
		gtk_dialog_run(GTK_DIALOG(self.dialog));
	}
	gtk_widget_hide(self.dialog);

	return self.new_keyid;
}

static void gnupg_window_create(void)
{
	GtkWidget *dialog;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *scrolledwin;
	GtkWidget *treeview;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *model;
	gchar *buf;

	/* dialog */
	buf = g_strdup_printf("%s - %s", _("Select secret key"), FORMAL_NAME);
	dialog = gtk_dialog_new_with_buttons
			(buf,
			 NULL,
			 GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
			 GTK_STOCK_OK, GTK_RESPONSE_OK,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	gtk_widget_set_size_request(dialog, 480, 240);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), WIN_BORDER);
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_widget_realize(dialog);

	vbox = GTK_DIALOG(dialog)->vbox;
	gtk_box_set_spacing(GTK_BOX(vbox), VSPACING);

	/* label */
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new
		(_("Please select the secret key. "
		   "A public key for this secret key is used for the protection of ID information. "
		   "You are asked to input the passphrase of this secret key when you start Mnid."));
	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_FILL);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 0);

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
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_POINTER); /* for key */
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

	TREE_VIEW_APPEND_TEXT_COLUMN(_("Size"), COL_ALGO, 72);
	TREE_VIEW_APPEND_TEXT_COLUMN(_("Key ID"), COL_KEYID, 76);
	TREE_VIEW_APPEND_TEXT_COLUMN(_("Name"), COL_NAME, 130);
	TREE_VIEW_APPEND_TEXT_COLUMN(_("Address"), COL_EMAIL, 130);
	TREE_VIEW_APPEND_TEXT_COLUMN(_("Val"), COL_VALIDITY, 20);

#undef TREE_VIEW_APPEND_TEXT_COLUMN

	/* signal connect */
	g_signal_connect(G_OBJECT(treeview), "row-activated", 
			 G_CALLBACK(gnupg_window_keylist_activated), NULL);
	g_signal_connect(G_OBJECT(dialog), "key_press_event",
			 G_CALLBACK(gnupg_window_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(gnupg_window_response), NULL);
	g_signal_connect(G_OBJECT(dialog), "destroy",
			 G_CALLBACK(gnupg_window_destroy), NULL);

	/* pointer */
	self.dialog	= dialog;
	self.treeview	= treeview;
	self.model	= GTK_TREE_MODEL(model);
}

static GtkTreePath *gnupg_window_set_row(gpgme_key_t key)
{
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	const gchar *text[N_COL_TITLES];
	const gchar *s;
	gchar *tmp;

	if(!key->can_encrypt)
		return NULL;

	tmp = g_strdup_printf
		("%du/%s", key->subkeys->length,
		 gpgme_pubkey_algo_name(key->subkeys->pubkey_algo));
	text[COL_ALGO] = tmp;

	s = key->subkeys->keyid;
	if(strlen(s) == 16)
		s += 8;
	text[COL_KEYID] = s;

	s = key->uids->name;
	text[COL_NAME] = s;

	s = key->uids->email;
	text[COL_EMAIL] = s;

	switch(key->uids->validity){
	case GPGME_VALIDITY_UNDEFINED:
		s = "q";
		break;
	case GPGME_VALIDITY_NEVER:
		s = "n";
		break;
	case GPGME_VALIDITY_MARGINAL:
		s = "m";
		break;
	case GPGME_VALIDITY_FULL:
		s = "f";
		break;
	case GPGME_VALIDITY_ULTIMATE:
		s = "u";
		break;
	case GPGME_VALIDITY_UNKNOWN:
	default:
		s = "?";
		break;
	}
	text[COL_VALIDITY] = s;

	gtk_list_store_append(GTK_LIST_STORE(self.model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(self.model), &iter,
			   COL_ALGO,	text[COL_ALGO],
			   COL_KEYID,	text[COL_KEYID],
			   COL_NAME,	text[COL_NAME],
			   COL_EMAIL,	text[COL_EMAIL],
			   COL_VALIDITY,text[COL_VALIDITY],
			   COL_POINTER,	key,
			   -1);
	g_free(tmp);

	path = gtk_tree_model_get_path(self.model, &iter);

	return path;
}

static void gnupg_window_destroy_key(gpointer data)
{
	gpgme_key_t key = (gpgme_key_t)data;
	gpgme_key_release(key);
	key = NULL;
}

static void gnupg_window_create_keylist(void)
{
	gpgme_ctx_t ctx = NULL;
	gpgme_error_t err;
	gpgme_key_t key;
	gpgme_key_t r_key;
	GtkTreePath *path;

	gtk_list_store_clear(GTK_LIST_STORE(self.model));

	err = gpgme_new(&ctx);
	g_assert(!err);

	err = gpgme_op_keylist_start(ctx, "", 0);
	if(err){
		debug_print("gpgme_op_keylist_start failed: %s\n",
				gpgme_strerror(err));
		gpgme_release(ctx);
		return;
	}

	while(!(err = gpgme_op_keylist_next(ctx, &key))){
		err = gpgme_get_key(ctx, key->subkeys->fpr, &r_key, TRUE);
		if(gpgme_err_code(err) == GPG_ERR_NO_ERROR){
			path = gnupg_window_set_row(key);
			if(path &&
			   !strcmp2(key->subkeys->keyid, self.cur_keyid)){
				gtk_tree_view_set_cursor
					(GTK_TREE_VIEW(self.treeview),
				 	path, NULL, FALSE);
			}
			gtk_tree_path_free(path);
		}
		key = NULL;
	}

	if(gpgme_err_code(err) != GPG_ERR_EOF){
		gpgme_op_keylist_end(ctx);
	}

	if(ctx)
		gpgme_release(ctx);
}

/* callback functions */
static gint gnupg_window_key_pressed(GtkWidget *widget, GdkEventKey *event,
				gpointer data)
{
	if(event && event->keyval == GDK_Escape){
		gtk_dialog_response(GTK_DIALOG(self.dialog),
				    GTK_RESPONSE_CANCEL);
		return TRUE;
	}
	return FALSE;
}

static void gnupg_window_response(GtkDialog *dialog, gint response_id,
				    gpointer data)
{
	switch(response_id)
	{
	case GTK_RESPONSE_OK:
		gnupg_window_ok(NULL, data);
		break;
	case GTK_RESPONSE_CANCEL:
		gnupg_window_cancel(NULL, data);
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

static void gnupg_window_ok(GtkWidget *widget, gpointer data)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gpgme_key_t key;

	gtk_tree_view_get_cursor(GTK_TREE_VIEW(self.treeview), &path, NULL);
	if(!path){
		debug_print("nothing selected\n");
		return;
	}
	gtk_tree_model_get_iter(self.model, &iter, path);
	gtk_tree_path_free(path);

	gtk_tree_model_get(self.model, &iter,
			   COL_POINTER, &key,
			   -1);
	if(key){
		if(key->uids->validity < GPGME_VALIDITY_FULL){
			return;
		}
		self.new_keyid = g_strdup(key->subkeys->keyid);
	}

	self.is_finished = TRUE;
}

static void gnupg_window_cancel(GtkWidget *widget , gpointer data)
{
	self.new_keyid = NULL;
	self.is_finished = TRUE;
}

static void gnupg_window_keylist_activated(GtkTreeView *treeview,
					   GtkTreePath *path,
					   GtkTreeViewColumn *column,
					   gpointer data)
{
	gtk_dialog_response(GTK_DIALOG(self.dialog), GTK_RESPONSE_OK);
}

static void gnupg_window_destroy(GtkWidget *widget, gpointer data)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	gpgme_key_t key;

	path = gtk_tree_path_new_first();
	while(gtk_tree_model_get_iter(self.model, &iter, path)){
		gtk_tree_model_get(self.model, &iter,
				   COL_POINTER, &key,
				   -1);
		/* free key */
		if(key)
			gnupg_window_destroy_key(key);
		gtk_tree_path_next(path);
	}
}
#endif /* USE_GPGME */
