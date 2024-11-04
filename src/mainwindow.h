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

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

typedef struct _MainWindow	MainWindow;

#include "id.h"

typedef enum
{
	M_COL_NAME,
	M_COL_UID,
	M_COL_POINTER,
	M_COLS,
} MainColumnType;


struct _MainWindow
{
	GtkWidget *window;
	GtkWidget *treeview;
	GtkWidget *menubar;
	GtkWidget *toolbar;
	GtkWidget *id_popup;
	GtkWidget *statusbar;

	GtkWidget *new_btn;
	GtkWidget *edit_btn;
	GtkWidget *copy_btn;
	GtkWidget *del_btn;

	GtkWidget *quit_btn;

	GtkTreeModel *model;

	guint context_id;

	gboolean is_updated;
};


MainWindow *main_window_create		(void);
void main_window_set_dialog		(MainWindow	*mainwin);

void main_window_get_size		(MainWindow	*mainwin);
void main_window_get_position		(MainWindow	*mainwin);

void main_window_save			(MainWindow	*mainwin,
					 gboolean	 is_force);

#endif /* __MAINWINDOW_H__ */
