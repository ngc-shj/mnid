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

#ifndef __PREFS_COMMON_H__
#define __PREFS_COMMON_H__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defines.h"

#include <glib.h>

typedef struct _PrefsCommon	PrefsCommon;

#include "mainwindow.h"
#include "id.h"

struct _PrefsCommon
{
	/* position and size */
	gint mainwin_x;
	gint mainwin_y;
	gint mainwin_width;
	gint mainwin_height;

	gint toolbar_style;
	gboolean show_statusbar;

	/* Privacy */
	gboolean use_encrypt;
	EncryptType encrypt_type;

	/* GnuPG */
	gchar *keyid;

	/* Other */
	gchar *uri_cmd;

	/* ID List */
	GSList *idlist;
	/*
	gboolean id_col_visible[M_COLS];
	gint id_col_pos[M_COLS];
	gint id_col_size[M_COLS];
	*/
	
	/* update flag */
	gboolean is_updated;
};

extern PrefsCommon prefs_common;

void prefs_common_read_config	(void);
void prefs_common_write_config	(void);
void prefs_common_open		(MainWindow		*mainwin);

#endif /* __PREFS_COMMON_H__ */
