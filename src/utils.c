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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#include "utils.h"

#define BUFSIZE	8192

extern gboolean debug_mode;

/* with NULL pointer check */
gint strcmp2(const gchar *s1, const gchar *s2)
{
	if(s1 == NULL || s2 == NULL)
		return -1;
	else
		return strcmp(s1, s2);
}

gchar *strretchomp(gchar *str)
{
	register gchar *s;

	if(!*str) return str;

	for(s = str + strlen(str) -1;
	    s >= str && (*s == '\n' || *s == '\r');
	    s--)
		*s = '\0';

	return str;
}

gchar *strcasestr(const gchar *haystack, const gchar *needle)
{
	register size_t haystack_len, needle_len;

	haystack_len = strlen(haystack);
	needle_len = strlen(needle);

	if(haystack_len < needle_len || needle_len == 0)
		return NULL;

	while(haystack_len >= needle_len){
		if(!strncasecmp(haystack, needle, needle_len))
			return (gchar *)haystack;
		else{
			haystack++;
			haystack_len--;
		}
	}

	return NULL;
}

gchar *strncpy2(gchar *dest, const gchar *src, size_t n)
{
	register gchar c;
	gchar *s = dest;

	do{
		if(--n <= 0){
			*dest = '\0';
			return s;
		}
		c = *src++;
		*dest++ = c;
	}while(c != '\0');

	return s;
}

gchar *strescapehtml(const gchar *src)
{
	GString *str;
	guint pos;
	gchar *p;

	str = g_string_new(src);

	/* '&' -> '&amp;' */
	p = g_utf8_strchr(str->str, -1, '&');
	while(p){
		pos = p - str->str + 1;
		g_string_insert(str, pos, "amp;");
		p = g_utf8_strchr(str->str + pos, -1, '&');
	}

	/* '\n' -> '<br/>' */
	/*
	p = g_utf8_strchr(str->str, -1, '\n');
	while(p){
		pos = p - str->str;
		g_string_erase(str, pos, 1);
		g_string_insert(str, pos, "<br/>");
		p = g_utf8_strchr(str->str + pos + 5, -1, '\n');
	}
	*/

	return g_string_free(str, FALSE);
}

const gchar *get_rc_dir(void)
{
	static gchar *rc_dir = NULL;
	
	if(!rc_dir)
		rc_dir = g_build_filename(g_get_home_dir(), RC_DIR, NULL);

	return rc_dir;
}

FILE *safe_fopen(const gchar *file)
{
	struct stat lst, fst;
	FILE *fp;

	if(lstat(file, &lst))
		return NULL;

	if(!is_file_exist(file)){
		debug_print("file not exist.\n");
		return NULL;
	}

	if((fp = fopen(file, "rb")) == NULL){
		if(ENOENT != errno)
			FILE_OP_ERROR(file, "fopen");
		return NULL;
	}

	if(fstat(fileno(fp), &fst)){
		fclose(fp);
		return NULL;
	}

	if(lst.st_ino != fst.st_ino || lst.st_dev != fst.st_dev){
		fclose(fp);
		return NULL;
	}

	return fp;
}

gint safe_creat(const gchar *file)
{
	gint fd;

	if((fd = open(file, O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR)) < 0){
		FILE_OP_ERROR(file, "open");
		return -1;
	}

	return fd;
}

gboolean file_get_contents(FILE *fp, gchar **contents, gsize *length)
{
	GByteArray *array;
	gsize f_size = 0;
	gsize len;
	gchar buf[BUFSIZE];

	g_return_val_if_fail(fp != NULL, FALSE);

	array = g_byte_array_new();

	while((len = fread(buf, sizeof(gchar), sizeof(buf), fp)) > 0){
		if(ferror(fp))
			break;
		f_size += len;
		g_byte_array_append(array, buf, len);
	}

	if(ferror(fp)){
		FILE_OP_ERROR(__FUNCTION__, "fread");
		g_byte_array_free(array, TRUE);
		return FALSE;
	}

	buf[0] = '\0';
	g_byte_array_append(array, buf, 1);

	*contents = g_byte_array_free(array, FALSE);
	*length = f_size;

	return TRUE;
}

gboolean is_file_exist(const gchar *file)
{
	if(!file)
		return FALSE;

	if(g_file_test(file, G_FILE_TEST_IS_REGULAR))
		return TRUE;

	return FALSE;
}

gboolean is_dir_exist(const gchar *dir)
{
	if(!dir)
		return FALSE;

	if(g_file_test(dir, G_FILE_TEST_IS_DIR))
		return TRUE;

	return FALSE;
}

gint change_dir(const gchar *dir)
{
	if(chdir(dir) < 0){
		FILE_OP_ERROR(dir, "chdir");
		return -1;
	}

	return 0;
}

gint make_dir(const gchar *dir)
{
	if(mkdir(dir, S_IRWXU) < 0){
		FILE_OP_ERROR(dir, "mkdir");
		return -1;
	}
	if(chmod(dir, S_IRWXU) < 0)
		FILE_OP_ERROR(dir, "chmod");

	return 0;
}

gint execute_command_line(const gchar *cmdline)
{
	gchar **argv;
	int argc;
	GPid pid;
	gint status;

	debug_print("executing: %s\n", cmdline);
	
	g_shell_parse_argv(cmdline, &argc, &argv, NULL);
	if(g_spawn_async(NULL, argv, NULL,
			 G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
			 NULL, NULL, &pid, NULL)){
		g_strfreev(argv);
		return -1;
	}
	
	waitpid(pid, &status, 0);
	g_strfreev(argv);
	g_spawn_close_pid(pid);

	if(WIFEXITED(status))
		return WEXITSTATUS(status);
	else
		return -1;
}

gint open_uri(const gchar *uri, const gchar *cmdline)
{
	gchar buf[BUFSIZE];
	gchar *p;

	g_return_val_if_fail(uri != NULL, -1);

	if(cmdline &&
	   (p = strchr(cmdline, '%')) && *(p + 1) == 's' &&
	   !strchr(p + 2, '%'))
		g_snprintf(buf, sizeof(buf), cmdline, uri);
	else{
		if(cmdline)
			g_warning("Open URI command line is invalid "
				  "(there must be only one '%%s'): %s",
				  cmdline);
		g_snprintf(buf, sizeof(buf), DEFAULT_BROWSER_CMD, uri);
	}

	execute_command_line(buf);

	return 0;
}

gchar *elapsed_time(const GTime start, const GTime end)
{
	GDate *s_dt, *e_dt;
	gint days;
	gint seconds;
	gchar buf[256];
	GString *str;
	GDateMonth s_month, e_month;
	GDateDay s_day, e_day;
	gint s_time, e_time;

	s_dt = g_date_new();
	e_dt = g_date_new();
	str = g_string_new(NULL);

	g_date_set_time(s_dt, start);
	g_date_set_time(e_dt, end);

	days = g_date_days_between(s_dt, e_dt);	

	if(days < 0)
		return _("0 seconds");

	seconds = end - start;

	s_month = g_date_get_month(s_dt);
	e_month = g_date_get_month(e_dt);
	s_day = g_date_get_day(s_dt);
	e_day = g_date_get_day(e_dt);
	/* Umm... 1 leap seconds... */
	s_time = start % (24 * 3600);
	e_time = end % (24 * 3600);

	if(days >= 365)
		g_snprintf(buf, sizeof(buf), _("%d year(s)"), days / 365);
	else if(days >= 31 ||
		(s_month != e_month &&
		 (s_day < e_day || (s_day == e_day && s_time < e_time)))){
		gint months;

		months = (g_date_get_year(e_dt) - g_date_get_year(s_dt)) * 12
		       + g_date_get_month(e_dt) - g_date_get_month(s_dt);
		if(g_date_get_day(s_dt) >  g_date_get_day(e_dt))
			months--;
		g_snprintf(buf, sizeof(buf), _("%d month(s)"), months);
	}else if(days >= 7)
		g_snprintf(buf, sizeof(buf), _("%d week(s)"), days / 7);
	else if(days >= 1)
		g_snprintf(buf, sizeof(buf), _("%d day(s)"), days);
	else if(seconds >= 3600)
		g_snprintf(buf, sizeof(buf), _("%d hour(s)"), seconds / 3600);
	else if(seconds >= 60)
		g_snprintf(buf, sizeof(buf), _("%d minute(s)"), seconds / 60);
	else
		g_snprintf(buf, sizeof(buf), _("%d second(s)"), seconds);
	
	g_string_append_len(str, buf, strlen(buf));
	g_date_free(s_dt);
	g_date_free(e_dt);

	return g_string_free(str, FALSE);
}

guint password_quality_level(const gchar *password)
{
	glong len = g_utf8_strlen(password, -1);
	const gchar *p;
	gint upper_case = 0;
	gint symbols = 0;
	gint numbers = 0;
	gint strength = 0;

	for(p = password; *p; p = g_utf8_find_next_char(p, NULL)){
		gunichar uc = g_utf8_get_char(p);
		if(g_unichar_isdigit(uc)){
			numbers++;
		}else if(g_unichar_isupper(uc)){
			upper_case++;
		}else if(g_unichar_islower(uc)){
		}else if(g_unichar_isgraph(uc)){
			symbols++;
		}
	}

	if(len > 5)
		len = 5;
	if(numbers > 3)
		numbers = 3;
	if(symbols > 3)
		symbols = 3;
	if(upper_case > 3)
		upper_case = 3;

	strength = ((len * 10) - 20)
		+ (numbers * 10)
		+ (symbols * 15)
		+ (upper_case * 10);

	if(strength < 0)
		strength = 0;
	if(strength > 100)
		strength = 100;

	return strength;
}

void debug_print(const gchar *format, ...)
{
	va_list args;
	gchar buf[BUFSIZE];
	gchar *p;

	if(!debug_mode) return;

	va_start(args, format);
	g_vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	p = g_locale_from_utf8(buf, -1, NULL, NULL, NULL);
	fputs(p, stdout);
	g_free(p);
}
