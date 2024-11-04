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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "blowfish.h"
#include "utils.h"

static gchar *bf_key = NULL;

const gchar *blowfish_key_get(void)
{
	return bf_key;
}

gboolean blowfish_key_set(const gchar *key)
{
	if(!key)
		return FALSE;
	
	blowfish_key_free();

	bf_key = g_strdup(key);

	if(mlock(bf_key, strlen(bf_key)) == -1)
		debug_print("%% locking blowfish key failed.\n");

	return TRUE;
}

void blowfish_key_free(void)
{
	if(bf_key){
		munlock(bf_key, strlen(bf_key));
		g_free(bf_key);
		bf_key = NULL;
		debug_print("%% unlocking blowfish key.\n");
	}
}
