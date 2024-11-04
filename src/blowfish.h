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

#ifndef __MNID_BLOWFISH_H__
#define __MNID_BLOWFISH_H__

#include <glib.h>

#define	BLOWFISH_KEY_MAX	56

const gchar *blowfish_key_get			(void);
gboolean blowfish_key_set			(const gchar	*key);

void blowfish_key_free				(void);

#endif /* __MNID_BLOWFISH_H__ */
