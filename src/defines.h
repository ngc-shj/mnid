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

#ifndef __DEFINES_H__
#define __DEFINES_H__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#define	FORMAL_NAME		"Mnid"

#define RC_DIR			".mnid"
#define COMMON_RC		"mnidrc.xml"
#define	ID_LIST			"idlist.xml"
#define ICON_RC			"mnid.png"

#define WEBSITE_URI		"http://www.org3.net/"
#define DEFAULT_BROWSER_CMD	"mozilla %s"

#define DEFAULT_TITLE_FONT	"Helvetica 16"

#define BUFSIZE			8192

#define MAX_PWD_ENTRY_LENGTH	8
#define VSPACING                8
#define VSPACING_NARROW		4
#define WIN_BORDER              8
#define VBOX_BORDER             16
#define FRAME_VBOX_BORDER       8

#endif /* __DEFINES_H__ */
