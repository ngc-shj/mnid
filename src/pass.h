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

#ifndef __PASS_H__
#define __PASS_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <libxml/tree.h>

#define MNID_PASS_MAX		99
#define MNID_PASS_DEFAULT	6

typedef struct _PassInfo	PassInfo;

typedef enum
{
	PASS_UPPER,
	PASS_LOWER,
	PASS_NUMERIC,
	PASS_SYMBOL,
} PassCharacterType;

#define N_PASS_USING_CHAR	4

struct _PassInfo
{
	guint id;
	gchar *pass;
	gboolean use_char[N_PASS_USING_CHAR];
	/*
	gboolean upper;
	gboolean lower;
	gboolean numeric;
	gboolean symbol;
	*/
	gboolean avoid_ambiguous;
	gint digit;

	GTime update_date;
};


PassInfo *pass_new			(void);
PassInfo *pass_copy			(const PassInfo	*passinfo);
PassInfo *pass_copy_with_pass		(const PassInfo	*passinfo,
					 const gchar	*pass);
gboolean pass_update_value		(PassInfo	*passinfo,
					 const gchar	*pass,
					 const gboolean  use_char[],
					 const gboolean  avoid_ambiguous,
					 const gint	 digit);
void pass_free				(PassInfo	*passinfo);

PassInfo *pass_read_config		(xmlNode	*node);
GSList *pass_xmlnode_to_passlist	(const xmlNode	*node);
void pass_write_config			(xmlNode	*node,
					 const PassInfo	*passinfo);

#endif /* __PASS_H__ */
