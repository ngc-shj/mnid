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

#ifndef __PREFS_ID_H__
#define __PREFS_ID_H__

#ifdef HAVE_CONFIG_H_
#  include "config.h"
#endif

#include <glib.h>
#include "id.h"

typedef enum
{
	PREFS_ID_MODE_NEW,
	PREFS_ID_MODE_EDIT,
	PREFS_ID_MODE_COPY,
}PrefsIdMode;

gboolean prefs_id_read_config	(void);
void prefs_id_write_config	(void);

IDinfo *prefs_id_open		(IDinfo		*idinfo,
				 PrefsIdMode	 mode);

#endif /* __PREFS_ID_H__ */
