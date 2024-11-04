/*
 * mnid
 *
 * Copyright (C) 2005 NOGUCHI Shoji
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
 * Copyright (C) 1999-2005 Hiroyuki Yamamoto
 *
 * passphrase.c - GTK+ based passphrase callback
 *      Copyright (C) 2001 Werner Koch (dd9jn)
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if USE_GPGME

#include "defines.h"

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "gnupg_crypt.h"
#include "prefs_common.h"
#include "utils.h"

static gboolean passphrase_key_pressed	(GtkWidget	*widget,
					 GdkEventKey	*event,
					 gpointer	 data);
static void passphrase_response		(GtkDialog	*dialog,
					 gint		 response_id,
					 gpointer	 data);
static void passphrase_activate		(GtkWidget	*widget,
					 gpointer	 data);
static gchar* passphrase_mbox		(const gchar	*uid_hint,
					 const gchar	*pass_hint,
					 gint		 prev_was_bad);
static GtkWidget *create_description	(const gchar	*uid_hint,
					 const gchar	*pass_hint,
					 gint		prev_was_bad);


static gchar* passphrase_mbox (const gchar *uid_hint, const gchar *pass_hint,
			       gint prev_was_bad)
{
	gchar *the_passphrase = NULL;
	GtkWidget *dialog;
	GtkWidget *vbox;
	GtkWidget *pass_entry;
	gchar *buf;

	/* dialog */
	buf = g_strdup_printf("%s - %s", _("Passphrase"), "Mnid");
	dialog = gtk_dialog_new_with_buttons
			(buf,
			 NULL,
			 GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
			 GTK_STOCK_OK, GTK_RESPONSE_OK,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 NULL);
	g_free(buf);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	gtk_widget_set_size_request(dialog, 450, -1);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), WIN_BORDER);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	g_signal_connect(G_OBJECT(dialog), "key_press_event",
			 G_CALLBACK(passphrase_key_pressed), dialog);
	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(passphrase_response), dialog);

	vbox = GTK_DIALOG(dialog)->vbox;
	gtk_box_set_spacing(GTK_BOX(vbox), VSPACING);

	if (uid_hint || pass_hint) {
		GtkWidget *label;
		label = create_description (uid_hint, pass_hint, prev_was_bad);
		gtk_widget_show(label);
		gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 0);
	}

	/* passphrase */
	pass_entry = gtk_entry_new();
	gtk_widget_show(pass_entry);
	gtk_box_pack_start(GTK_BOX(vbox), pass_entry, FALSE, FALSE, 0);
	gtk_entry_set_visibility(GTK_ENTRY(pass_entry), FALSE);
	gtk_widget_grab_focus(pass_entry);

	g_signal_connect(G_OBJECT(pass_entry), "activate",
			 G_CALLBACK(passphrase_activate), dialog);

	/* run dialog */
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK){
		const gchar *entry_text =
			gtk_entry_get_text(GTK_ENTRY(pass_entry));
		if (entry_text)
			the_passphrase = g_strdup (entry_text);
	}
	gtk_widget_destroy (dialog);

	return the_passphrase;
}

static gboolean passphrase_key_pressed(GtkWidget *widget, GdkEventKey *event,
					   gpointer data)
{
	if (event && event->keyval == GDK_Escape){
		gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_CANCEL);
		return TRUE;
	}
	return FALSE;
}

static void passphrase_response(GtkDialog *dialog, gint response_id,
				gpointer data)
{
	switch(response_id)
	{
	case GTK_RESPONSE_OK:
		break;
	case GTK_RESPONSE_CANCEL:
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_CANCEL);
		break;
	case GTK_RESPONSE_NONE:
		break;
	default:
		break;

	}
}

static void passphrase_activate(GtkWidget *widget, gpointer data)
{
	gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_OK);
}

static gint linelen (const gchar *s)
{
	gint i;

	for (i = 0; *s && *s != '\n'; s++, i++)
		;

	return i;
}

static GtkWidget *create_description (const gchar *uid_hint,
		const gchar *pass_hint, gint prev_was_bad)
{
	const gchar *uid = NULL, *info = NULL;
	gchar *buf;
	GtkWidget *label;

	if (!uid_hint)
		uid = _("[no user id]");
	else
		uid = uid_hint;
	if (!pass_hint)
		info = "";
	else
		info = pass_hint;

	buf = g_strdup_printf (_("%sPlease enter the passphrase for:\n\n"
				 "  %.*s  \n"
				 "(%.*s)\n"),
				 prev_was_bad ?
				 _("Bad passphrase! Try again...\n\n") : "",
				 linelen (uid), uid, linelen (info), info);

	label = gtk_label_new (buf);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	g_free (buf);

	return label;
}

gpgme_error_t gtk_gpgme_passphrase_cb (void *hook, const char *uid_hint,
		const char *passphrase_info, int prev_was_bad, int fd)
{
	const char *pass;

	debug_print ("%% requesting passphrase :`%s'\n", uid_hint);
	pass = passphrase_mbox (uid_hint, passphrase_info, prev_was_bad);
	if (!pass) {
		debug_print ("%% cancel passphrase entry\n");
		write(fd, "\n", 1);
		return GPG_ERR_CANCELED;
	}
	else {
		debug_print ("%% sending passphrase\n");
	}
	write(fd, pass, strlen(pass));
	write(fd, "\n", 1);

	return GPG_ERR_NO_ERROR;
}
#endif /* USE_GPGME */
