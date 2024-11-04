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
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "about.h"
#include "prefs_common.h"
#include "utils.h"
#include "gtkutils.h"
#include "version.h"


static GtkWidget *about_dialog = NULL;

static void about_create			(void);
static gboolean key_pressed			(GtkWidget	*widget,
						 GdkEventKey *event);
static void response				(GtkDialog	*dialog,
						 gint		 response_id,
						 gpointer	 data);

void about_show(void)
{
	if (!about_dialog)
		about_create();

	gtk_dialog_run(GTK_DIALOG(about_dialog));
	gtk_widget_hide(about_dialog);
}

static void about_create(void)
{
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *icon;
	GtkWidget *scrolledwin;
	GtkWidget *text;
	GtkTextBuffer *buffer;
	GtkTextIter iter;

	gchar *icon_path;
	gchar *ver;

	/* dialog */
	about_dialog = gtk_dialog_new_with_buttons
			(_("About Mnid"),
			 NULL,
			 GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
			 GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
			 NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(about_dialog),
					GTK_RESPONSE_CLOSE);
	gtk_container_set_border_width(GTK_CONTAINER(about_dialog), WIN_BORDER);
	gtk_window_set_position(GTK_WINDOW(about_dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(about_dialog), 340, 550);
	gtk_widget_realize(about_dialog);

	g_signal_connect(G_OBJECT(about_dialog), "key_press_event",
			 G_CALLBACK(key_pressed), NULL);
	g_signal_connect(G_OBJECT(about_dialog), "response",
			 G_CALLBACK(response), NULL);

	vbox = GTK_DIALOG(about_dialog)->vbox;
	gtk_box_set_spacing(GTK_BOX(vbox), VSPACING);

	/* icon */
	icon_path = g_build_filename(PIXMAPS_DIR, ICON_RC, NULL);
	icon = gtk_image_new_from_file(icon_path);
	gtk_widget_show(icon);
	gtk_box_pack_start(GTK_BOX(vbox), icon, FALSE, FALSE, 0);
	g_free(icon_path);

	/* version */
	ver = g_strconcat(_("Mnid version "), VERSION, NULL);
	label = gtk_label_new(ver);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtkutils_widget_set_font(label, -1, PANGO_WEIGHT_BOLD,
				 PANGO_SCALE_LARGE);
	g_free(ver);

	/* copyright */
	label = gtk_label_new("Copyright (C) 2005 NOGUCHI Shoji");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	label = gtk_label_new("SPECIAL THANKS: h-suzuki@Niigata Univ.");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtkutils_widget_set_font(label, -1, -1, PANGO_SCALE_SMALL);
	
	/* scroll window */
	scrolledwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrolledwin);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
					    GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(vbox), scrolledwin, TRUE, TRUE, 0);

	text = gtk_text_view_new();
	gtk_widget_show(text);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text), 8);
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text), 8);
	gtk_container_add(GTK_CONTAINER(scrolledwin), text);
	gtkutils_widget_set_font(text, -1, -1, PANGO_SCALE_SMALL);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

	/* copyright - other programs */
	gtk_text_buffer_insert(buffer, &iter,
		_("I have copied a part of code from other programs. "
		  "I extend a special thank you to the following programs.\n\n"), -1);
	gtk_text_buffer_insert(buffer, &iter,
		_("Sylpheed !\n"
		  "  Copyright (C) 1999-2005 Hiroyuki Yamamoto\n"
#if USE_GPGME
		  "  Copyright (C) 2001 Werner Koch (dd9jn)\n"
#endif /* USE_GPGME */
		  "\n"), -1);

	/* GPL2 */
	gtk_text_buffer_insert(buffer, &iter,
		_("This program is free software; you can redistribute it and/or modify "
		  "it under the terms of the GNU General Public License as published by "
		  "the Free Software Foundation; either version 2, or (at your option) "
		  "any later version.\n\n"), -1);

	gtk_text_buffer_insert(buffer, &iter,
		_("This program is distributed in the hope that it will be useful, " 
		   "but WITHOUT ANY WARRANTY; without even the implied warranty of "
		   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
		   "See the GNU General Public License for more details.\n\n"), -1);

	gtk_text_buffer_insert(buffer, &iter,
		_("You should have received a copy of the GNU General Public License "
		   "along with this program; if not, write to the Free Software "
		   "Foundation, Inc., 59 Temple Place - Suite 330, Boston, "
		   "MA 02111-1307, USA."), -1);

	debug_print("about create...done\n");
}

static gboolean key_pressed(GtkWidget *widget, GdkEventKey *event)
{
	if (event && event->keyval == GDK_Escape){
		gtk_dialog_response(GTK_DIALOG(about_dialog),
				    GTK_RESPONSE_CLOSE);
	}
	return FALSE;
}

static void response(GtkDialog	*dialog, gint response_id, gpointer data)
{
	switch(response_id)
	{
	case GTK_RESPONSE_CLOSE:
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		gtk_dialog_response(GTK_DIALOG(about_dialog),
				    GTK_RESPONSE_CLOSE);
		break;
	case GTK_RESPONSE_NONE:
		break;
	default:
		break;
	}
}
