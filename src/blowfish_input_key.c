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
#include <string.h>

#include "blowfish_input_key.h"
#include "blowfish.h"
#include "utils.h"
#include "gtkutils.h"

static struct _BlowfishKeyWindow
{
	GtkWidget *dialog;
	
	GtkWidget *old_key_entry;
	GtkWidget *key_entry;
	GtkWidget *key_re_entry;
	GtkWidget *progress_bar;
	gchar **key;

	gboolean key_ack;
	gboolean is_finished;
} self;

static void blowfish_window_create		(void);
static void blowfish_window_ok			(void);
static void blowfish_window_cancel		(void);

/* callback functions */
static gboolean dialog_key_press_event_cb	(GtkWidget	*widget,
						 GdkEventKey	*event,
						 gpointer	 data);
static void dialog_response_cb			(GtkDialog	*dialog,
						 gint		 reponse_id,
						 gpointer	 data);
static void password_quality_meter_cb		(GtkEditable	*editable,
						 gpointer	 data);

static void key_entries_activate_cb		(void);

gboolean blowfish_input_key_open(gchar **key)
{
	if(!self.dialog)
		blowfish_window_create();

	self.key = key;
	self.key_ack = FALSE;
	self.is_finished = FALSE;

	if(blowfish_key_get())
		gtk_widget_set_sensitive(self.old_key_entry, TRUE);
	else
		gtk_widget_set_sensitive(self.old_key_entry, FALSE);

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
	GtkWidget *vbox2;
	GtkWidget *frame;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *old_key_entry;
	GtkWidget *key_entry;
	GtkWidget *key_re_entry;
	GtkWidget *progress_bar;
	gchar *buf;

	buf = g_strdup_printf("%s - %s", _("Setting Key"), FORMAL_NAME);
	dialog = gtk_dialog_new_with_buttons
			(buf,
			 NULL,
			 GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
			 GTK_STOCK_OK, GTK_RESPONSE_OK,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	/* gtk_widget_set_size_request(dialog, 180 * 3, 200); */
	gtk_container_set_border_width(GTK_CONTAINER(dialog), WIN_BORDER);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_widget_realize(dialog);

	vbox = GTK_DIALOG(dialog)->vbox;
	gtk_box_set_spacing(GTK_BOX(vbox), VSPACING);

	/* explain */
	label = gtk_label_new
		(_("This Key is used for the protection of ID information. "
		   "You are asked to input the Key when you start Mnid."));
	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_FILL);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	frame = gtk_frame_new(NULL);
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);

	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox2);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), FRAME_VBOX_BORDER);
	
	/* table */
	table = gtk_table_new(4, 2, FALSE);
	gtk_widget_show(table);
	gtk_box_pack_start(GTK_BOX(vbox2), table, FALSE, TRUE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(table), VSPACING_NARROW);
	gtk_table_set_col_spacings(GTK_TABLE(table), 8);

	/* current key entry */
	label = gtk_label_new(_("Current key"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label,
			 0, 1, 0, 1,
			 GTK_FILL, 0, 0, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	old_key_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(old_key_entry), BLOWFISH_KEY_MAX);
	gtk_widget_show(old_key_entry);
	gtk_table_attach(GTK_TABLE(table), old_key_entry,
			 1, 2, 0, 1,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);
        gtk_entry_set_visibility(GTK_ENTRY(old_key_entry), FALSE);

	/* new key entry */
	label = gtk_label_new(_("New key"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label,
			 0, 1, 1, 2,
			 GTK_FILL, 0, 0, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	key_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(key_entry), BLOWFISH_KEY_MAX);
	gtk_widget_show(key_entry);
	gtk_table_attach(GTK_TABLE(table), key_entry,
			 1, 2, 1, 2,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);
        gtk_entry_set_visibility(GTK_ENTRY(key_entry), FALSE);

	/* max length of key */
	label = gtk_label_new(_("Max length of key is 56 chars."));
	gtk_widget_show(label);
	gtkutils_widget_set_font(label, -1, -1, PANGO_SCALE_SMALL);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);
	gtk_table_attach(GTK_TABLE(table), label,
			 1, 2, 2, 3,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);

	/* new key re-entry */
	label = gtk_label_new(_("Re-enter new key"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label,
			 0, 1, 3, 4,
			 GTK_FILL, 0, 0, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	key_re_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(key_re_entry), BLOWFISH_KEY_MAX);
	gtk_widget_show(key_re_entry);
	gtk_table_attach(GTK_TABLE(table), key_re_entry,
			 1, 2, 3, 4,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);
        gtk_entry_set_visibility(GTK_ENTRY(key_re_entry), FALSE);

	/* key quality */
	frame = gtk_frame_new(_("Key quality"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);

	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox2);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), FRAME_VBOX_BORDER);
	
	progress_bar = gtk_progress_bar_new();
	gtk_widget_show(progress_bar);
	gtk_box_pack_start(GTK_BOX(vbox2), progress_bar, FALSE, TRUE, 0);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
	
	/* signals */
	g_signal_connect(G_OBJECT (dialog), "key_press_event",
			 G_CALLBACK (dialog_key_press_event_cb), NULL);
	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(dialog_response_cb), NULL);

	g_signal_connect(G_OBJECT(key_entry), "changed",
			 G_CALLBACK (password_quality_meter_cb), NULL);

	g_signal_connect(G_OBJECT (old_key_entry), "activate",
			 G_CALLBACK (key_entries_activate_cb), NULL);
	g_signal_connect(G_OBJECT (key_entry), "activate",
			 G_CALLBACK (key_entries_activate_cb), NULL);
	g_signal_connect(G_OBJECT (key_re_entry), "activate",
			 G_CALLBACK (key_entries_activate_cb), NULL);

	self.dialog		= dialog;
	self.old_key_entry	= old_key_entry;
	self.key_entry		= key_entry;
	self.key_re_entry	= key_re_entry;
	self.progress_bar	= progress_bar;
}

static gboolean dialog_key_press_event_cb(GtkWidget *widget, GdkEventKey *event,
					  gpointer data)
{
	if(event && event->keyval == GDK_Escape)
		blowfish_window_cancel();
	return FALSE;
}

static void password_quality_meter_cb(GtkEditable *editable, gpointer data)
{
	gchar *str = gtk_editable_get_chars(editable, 0, -1);
	guint strength;

	strength = password_quality_level(str);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(self.progress_bar),
				      (strength / 100.0));

	g_free(str);
}

static gboolean blowfish_window_check(void)
{
	const gchar *old_key1, *old_key2;
	const gchar *new_key1, *new_key2;
	gboolean old_key_match = FALSE;
	gboolean new_key_match = FALSE;

	/* compare old key */
	old_key1 = blowfish_key_get();
	old_key2 = gtk_entry_get_text(GTK_ENTRY(self.old_key_entry));

	if((!old_key1 || *old_key1 == '\0') &&
	   (!old_key2 || *old_key2 == '\0'))
		old_key_match = TRUE;
	else if(!strcmp2(old_key1, old_key2))
		old_key_match = TRUE;

	/* compare new key */
	new_key1 = gtk_entry_get_text(GTK_ENTRY(self.key_entry));
	new_key2 = gtk_entry_get_text(GTK_ENTRY(self.key_re_entry));

	if(!strcmp2(new_key1, new_key2))
		new_key_match = TRUE;

	return old_key_match & new_key_match;
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

		*self.key = g_strdup(key);
		key = NULL;

		self.key_ack = TRUE;
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

	gtk_entry_set_text(GTK_ENTRY(self.old_key_entry), "");
	gtk_entry_set_text(GTK_ENTRY(self.key_entry), "");
	gtk_entry_set_text(GTK_ENTRY(self.key_re_entry), "");
	gtk_widget_grab_focus(self.old_key_entry);
}

static void blowfish_window_cancel(void)
{
	self.key_ack = FALSE;
	self.is_finished = TRUE;
}

static void key_entries_activate_cb(void)
{
	gtk_dialog_response(GTK_DIALOG(self.dialog), GTK_RESPONSE_OK);
}
