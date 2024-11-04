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

#include "blowfish_key.h"
#include "blowfish.h"
#include "utils.h"

static struct _BlowfishKeyWindow
{
	GtkWidget *dialog;
	GtkWidget *key_entry;

	GtkResponseType key_ack;
	gboolean is_finished;
} self;

static void blowfish_window_create		(void);
static void blowfish_window_ok			(void);
static void blowfish_window_cancel		(void);

/* callback functions */
static gboolean dialog_key_pressed_event_cb	(GtkWidget	*widget,
						 GdkEventKey	*event,
						 gpointer	 data);
static void dialog_response_cb			(GtkDialog	*dialog,
						 gint		 reponse_id,
						 gpointer	 data);
static void key_entries_activate_cb		(void);

GtkResponseType blowfish_key_open(void)
{
	if(!self.dialog)
		blowfish_window_create();

	self.key_ack = GTK_RESPONSE_NONE;
	self.is_finished = FALSE;

	while(self.is_finished == FALSE){
		gtk_dialog_run(GTK_DIALOG(self.dialog));
	}
	gtk_widget_hide(self.dialog);

	return self.key_ack;
}

static void blowfish_window_create(void)
{
	GtkWidget *dialog;
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *key_entry;
	gchar *buf;

	buf = g_strdup_printf("%s - %s", _("Input Blowfish Key"), FORMAL_NAME);
	dialog = gtk_dialog_new_with_buttons
			(buf,
			 NULL,
			 GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
			 GTK_STOCK_OK, GTK_RESPONSE_OK,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	/* gtk_widget_set_size_request(dialog, 180 * 3, 120); */
	gtk_container_set_border_width(GTK_CONTAINER(dialog), WIN_BORDER);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_widget_realize(dialog);

	vbox = GTK_DIALOG(dialog)->vbox;
	gtk_box_set_spacing(GTK_BOX(vbox), VSPACING);

	/* key entry */
	table = gtk_table_new(2, 2, FALSE);
	gtk_widget_show(table);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(table), VSPACING_NARROW);
	gtk_table_set_col_spacings(GTK_TABLE(table), 8);

	label = gtk_label_new(_("Key"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label,
			 0, 1, 0, 1,
			 GTK_FILL, 0, 0, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	key_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(key_entry), BLOWFISH_KEY_MAX);
	gtk_widget_show(key_entry);
	gtk_table_attach(GTK_TABLE(table), key_entry,
			 1, 2, 0, 1,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);
        gtk_entry_set_visibility(GTK_ENTRY(key_entry), FALSE);

	g_signal_connect(G_OBJECT(key_entry), "activate",
			 G_CALLBACK(key_entries_activate_cb), NULL);
	g_signal_connect(G_OBJECT(dialog), "key_press_event",
			 G_CALLBACK(dialog_key_pressed_event_cb), NULL);
	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(dialog_response_cb), NULL);

	self.dialog		= dialog;
	self.key_entry		= key_entry;
}

static gboolean dialog_key_pressed_event_cb(GtkWidget *widget,
					    GdkEventKey *event, gpointer data)
{
	if(event && event->keyval == GDK_Escape)
		blowfish_window_cancel();
	return FALSE;
}

static gboolean blowfish_window_check(void)
{
	const gchar *key;

	key = gtk_entry_get_text(GTK_ENTRY(self.key_entry));

	if(!key || *key == '\0')
		return FALSE;

	return TRUE;
}

static void dialog_response_cb(GtkDialog *dialog, gint response_id,
		       		  gpointer data)
{
	switch(response_id)
	{
	case GTK_RESPONSE_OK:
		blowfish_window_ok();
		break;
	case GTK_RESPONSE_CANCEL:
		blowfish_window_cancel();
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

static void blowfish_window_ok(void)
{
	if(blowfish_window_check()){
		const gchar *key;

		key = gtk_entry_get_text(GTK_ENTRY(self.key_entry));

		blowfish_key_set(key);

		self.key_ack = GTK_RESPONSE_OK;
		self.is_finished = TRUE;
	}else{
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new(NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_OK,
						_("Incorrect key"));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}

	gtk_entry_set_text(GTK_ENTRY(self.key_entry), "");
	gtk_widget_grab_focus(self.key_entry);
}

static void blowfish_window_cancel(void)
{
	self.key_ack = GTK_RESPONSE_CANCEL;
	self.is_finished = TRUE;
}

static void key_entries_activate_cb(void)
{
	gtk_dialog_response(GTK_DIALOG(self.dialog), GTK_RESPONSE_OK);
}
