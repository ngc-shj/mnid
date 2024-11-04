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
 * Copyright (C) 1999-2004 Hiroyuki Yamamoto
 *
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_OP_ERROR(file, func)	\
{					\
	fprintf(stderr, "%s: ", file);	\
	perror(func);			\
}

gint strcmp2			(const gchar	*s1,
				 const gchar	*s2);
gchar *strretchomp		(gchar		*str);
gchar *strcasestr		(const gchar	*haystack,
				 const gchar	*needle);
gchar *strncpy2			(gchar		*dest,
				 const gchar	*src,
				 size_t		 n);

gchar *strescapehtml		(const gchar	*src);

const gchar *get_rc_dir		(void);

FILE *safe_fopen		(const gchar	*file);
gint safe_creat			(const gchar	*file);
gboolean file_get_contents	(FILE		*fp,
				 gchar	       **contents,
				 gsize		*length);
gboolean is_file_exist		(const gchar	*file);
gboolean is_dir_exist		(const gchar	*dir);

gint change_dir			(const gchar	*dir);
gint make_dir			(const gchar	*dir);

/* process execution */
gint execute_command_line	(const gchar	*cmdline);

/* open URI */
gint open_uri			(const gchar	*uri,
				 const gchar	*cmdline);

/* elapsed time */
gchar *elapsed_time		(const GTime	 start,
				 const GTime	 end);

guint password_quality_level	(const gchar	*password);

void debug_print	(const gchar	*format, ...) G_GNUC_PRINTF(1, 2);

#endif /* __UTILS_H__ */
