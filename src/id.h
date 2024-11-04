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

#ifndef __ID_H__
#define __ID_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <libxml/tree.h>

#include "pass.h"

typedef struct _IDinfo		IDinfo;

struct _IDinfo
{
	gchar *name;
	gchar *uid;
	PassInfo *passinfo;
	gchar *uri;
	gchar *remark;

	GTime create_date;
	GTime update_date;

	GSList *pass_list;
};

typedef enum
{
	ENCRYPT_TYPE_NONE       = -1,
	ENCRYPT_TYPE_BLOWFISH   =  0,
#if USE_GPGME
	ENCRYPT_TYPE_GNUPG      =  2,
#endif /* USE_GPGME */
} EncryptType;

/* read / write config */
gboolean id_read_config			(GSList	       **idlist,
					 EncryptType	 encrypt_type,
					 gpointer	 data);
void id_write_config			(GSList		*idlist,
					 EncryptType	 encrypt_type,
					 gpointer	 data);

IDinfo *id_new				(void);
IDinfo *id_copy				(const IDinfo	*idinfo);
void id_update_value			(IDinfo		*idinfo,
					 const gchar	*name,
					 const gchar	*uid,
					 const gchar	*uri,
					 const gchar	*remark);
void id_free				(IDinfo		*idinfo);

#endif /* __ID_H__ */
