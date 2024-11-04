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

#ifndef __PREFS_H__
#define __PREFS_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>

typedef struct _PrefParam	PrefParam;
typedef struct _PrefFile	PrefFile;
typedef struct _PrefsDialog	PrefsDialog;

typedef enum
{
	P_STRING,
	P_INT,
	P_BOOL,
	P_OTHER,
} PrefType;

struct _PrefFile
{
	FILE *fp;
	gchar *path;
	gchar *tmppath;
};

typedef gboolean (*DataSetFunc)	(PrefParam *pparam);
typedef void (*WidgetSetFunc)	(PrefParam *pparam);

struct _PrefParam
{
	gchar		*name;
	gchar		*defval;
	gpointer	 data;
	PrefType	 type;
	GtkWidget      **widget;
	DataSetFunc	 data_set_func;
	WidgetSetFunc	 widget_set_func;
};

struct _PrefsDialog
{
	GtkWidget *window;	/* GtkDialog */
	GtkWidget *notebook;

	gpointer parent;
	gboolean is_finished;
};

#define SET_TOGGLE_SENSITIVE(toggle, target) \
{ \
	gtk_widget_set_sensitive(target, FALSE); \
	g_signal_connect(G_OBJECT(toggle), "toggled", \
			 G_CALLBACK(prefs_button_toggled), target); \
}

void prefs_read_config			(PrefParam	*param,
					 const gchar	*label,
					 const gchar	*rcfile);
void prefs_config_parse_one_line	(PrefParam	*param,
					 const gchar	*buf);
void prefs_write_config			(PrefParam	*param,
					 const gchar	*label,
					 const gchar	*rcfile);

PrefFile *prefs_file_open		(const gchar	*path);
gint prefs_file_write_param		(PrefFile	*pfile,
					 PrefParam	*param);
gint prefs_file_close			(PrefFile	*pfile);
gint prefs_file_close_revert		(PrefFile	*pfile);

void prefs_set_default			(PrefParam	*param);
void prefs_free				(PrefParam	*param);

void prefs_dialog_create		(PrefsDialog	*dialog);
void prefs_dialog_destroy		(PrefsDialog	*dialog);

void prefs_button_toggled		(GtkToggleButton	*toggle_btn,
					 GtkWidget		*widget);

void prefs_set_dialog			(PrefParam	*param);
gboolean prefs_set_data_from_dialog	(PrefParam	*param);
void prefs_set_dialog_to_default	(PrefParam	*param);

/* entry */
gboolean prefs_set_data_from_entry	(PrefParam	*pparam);
void prefs_set_entry_from_data		(PrefParam	*pparam);
/* toggle */
gboolean prefs_set_data_from_toggle	(PrefParam	*pparam);
void prefs_set_toggle_from_data		(PrefParam	*pparam);
/* combo */
gboolean prefs_set_data_from_combo_box	(PrefParam	*pparam);
void prefs_set_combo_box_from_data	(PrefParam	*pparam);


#endif /* __PREFS_H__ */
