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
 * passphrase.c - GTK+ based passphrase callback
 *      Copyright (C) 2001 Werner Koch (dd9jn)
 */

#ifndef GNUPG_PASS_H
#define GNUPG_PASS_H

#include <glib.h>
#include <gpgme.h>

gpgme_error_t gtk_gpgme_passphrase_cb	(void		*hook,
					 const char	*uid_hint,
					 const char	*passphrase_hint,
					 int		 prev_was_bad,
					 int		fd);

#endif /* GNUPG_PASS_H */
