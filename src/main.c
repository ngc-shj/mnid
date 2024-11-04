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

/*
 * We have copied a part of code from the following other programs.
 *
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2005 Hiroyuki Yamamoto
 *
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkrc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/resource.h>
#include <locale.h>

#include "main.h"
#include "mainwindow.h"
#include "prefs_common.h"
#include "prefs_id.h"
#include "blowfish.h"
#include "utils.h"
#include "version.h"

gboolean debug_mode = FALSE;

static void rlimit_init				(void);
static void parse_cmd_opt			(gint		 argc,
						 gchar		*argv[]);
static gboolean prohibit_duplicate_launch	(void);
static void remove_lock_file			(void);
static gchar *get_lock_filename			(void);

#define MAKE_DIR_IF_NOT_EXIST(dir) \
{ \
	if(!is_dir_exist(dir)) { \
		if(is_file_exist(dir)) { \
			GtkWidget *dialog; \
			dialog = gtk_message_dialog_new(NULL, \
					GTK_DIALOG_MODAL, \
					GTK_MESSAGE_ERROR, \
					GTK_BUTTONS_OK, \
					_("File '%s' already exists.\n" \
					  "Can't create folder."), dir); \
			gtk_dialog_run(GTK_DIALOG(dialog)); \
			gtk_widget_destroy(dialog); \
			remove_lock_file(); \
			return 1; \
		} \
		if(make_dir(dir) < 0){ \
			remove_lock_file(); \
			return 1; \
		} \
	} \
}

int main(int argc, char *argv[])
{
	MainWindow *mainwin;
	gchar *userrc;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALE_DIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);

	rlimit_init();

	parse_cmd_opt(argc, argv);

	gtk_set_locale();
	gtk_init(&argc, &argv);

	if(!prohibit_duplicate_launch())
		return -1;

	gdk_rgb_init();
	gtk_widget_set_default_colormap(gdk_rgb_get_cmap());

	/* parse gtkrc files */
	userrc = g_build_filename(g_get_home_dir(), ".gtkrc-2.0", NULL);
	gtk_rc_parse(userrc);
	g_free(userrc);

	userrc = g_build_filename(g_get_home_dir(), ".gtk", ".gtkrc-2.0", NULL);
	gtk_rc_parse(userrc);
	g_free(userrc);

	/* change dir to home  */
	if(change_dir(g_get_home_dir()) < 0){
		remove_lock_file();
		return 1;
	}

	/* backup if old rc file exists */
	if(is_file_exist(RC_DIR)){
		if(rename(RC_DIR, RC_DIR ".bak") < 0)
			FILE_OP_ERROR(RC_DIR, "rename");
	}
	MAKE_DIR_IF_NOT_EXIST(RC_DIR);

	prefs_common_read_config();
	if(!prefs_id_read_config()){
		remove_lock_file();
		return -1;
	}

	mainwin = main_window_create();
	main_window_set_dialog(mainwin);

	gtk_main();

	return 0;
}

static void rlimit_init(void)
{
	struct rlimit core;

	core.rlim_cur = 0;
	core.rlim_max = 0;
	setrlimit(RLIMIT_CORE, &core);
}

/* Fix GLib 2.6.x */
static void parse_cmd_opt(gint argc, gchar *argv[])
{
	gint c;
	struct option long_options[] = {
		{"debug", 2, 0, 'd'},
		{0, 0, 0, 0}
	};

	while(1){
		c = getopt_long(argc, argv, "d:", long_options, NULL);
		if(c < 0) break;
		
		switch(c)
		{
		case 'd':
			debug_mode = TRUE;
			break;
		}
	}
}

void app_will_exit(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	/* save all state before exiting */
	main_window_get_position(mainwin);
	prefs_common_write_config();

	/* free all memories */
	blowfish_key_free();

	/* delete temporary files */
	remove_lock_file();

	gtk_main_quit();
}

static gboolean prohibit_duplicate_launch(void)
{
	gchar *lock_file;
	gint fd;

	lock_file = get_lock_filename();

	if((fd = safe_creat(lock_file)) < 0){
		return FALSE;
	}
	close(fd);

	return TRUE;
}

static gchar *get_lock_filename(void)
{
	static gchar *filename = NULL;

	if(!filename){
		gchar *file = g_strconcat(g_get_prgname(), "_",
					  g_get_user_name(), NULL);
		filename = g_build_filename(g_get_tmp_dir(), file, NULL);
		g_free(file);
	}
	return filename;
}

static void remove_lock_file(void)
{
	gchar *lock_file;

	lock_file = get_lock_filename();

	if(unlink(lock_file) < 0)
		FILE_OP_ERROR(lock_file, "unlink");
}
