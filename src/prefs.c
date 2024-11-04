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
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <unistd.h>

#include "prefs.h"
#include "prefs_common.h"
#include "utils.h"
#include "version.h"

static void prefs_config_parse_node	(PrefParam	*param,
					 xmlNode	*node);
static gint prefs_node_write_param	(xmlNode	*node,
					 PrefParam	*param);
	
void prefs_read_config(PrefParam *param, const gchar *label,
		       const gchar *rcfile)
{
	FILE *fp;
	xmlDoc *doc;
	xmlNode *cur;
	gchar *rcpath;
	gchar *f_data = NULL;
	size_t f_size;
	gboolean is_found;

	g_return_if_fail(param != NULL);
	g_return_if_fail(label != NULL);
	g_return_if_fail(rcfile != NULL);

	debug_print("Reading configuration...\n");
	
	prefs_set_default(param);

	rcpath = g_build_filename(get_rc_dir(), rcfile, NULL);
	if(!(fp = safe_fopen(rcpath))){
		g_free(rcpath);
		return;
	}
	g_free(rcpath);

	if(!file_get_contents(fp, &f_data, &f_size)){
		fclose(fp);
		return;
	}
	fclose(fp);

	doc = xmlReadMemory(f_data, f_size, NULL, NULL, 0);
	if(!doc){
		g_warning("Can't parse.\n");
		return;
	}

	cur = xmlDocGetRootElement(doc);
	is_found = FALSE;
	for(cur = cur->children; cur; cur = cur->next){
		if(cur->type == XML_ELEMENT_NODE &&
		   !strcasecmp(cur->name, label)){
			debug_print("Found %s\n", label);
			is_found = TRUE;
			break;
		}
	}

	if(is_found)
		for(cur = cur->children; cur; cur = cur->next)
			if(cur->type == XML_ELEMENT_NODE)
				prefs_config_parse_node(param, cur);

	debug_print("Finished reading configuration.\n");

	xmlFreeDoc(doc);
}

static void prefs_config_parse_node(PrefParam *param, xmlNode *node)
{
	gint i;
	gint name_len;
	xmlNode *child;
	const gchar *value;

	for(i = 0; param[i].name; i++)
	{
		name_len = strlen(param[i].name);
		if(strncasecmp(node->name, param[i].name, name_len))
			continue;
		child = node->children;
		value = child ? XML_GET_CONTENT(child)
			      : NULL;

		debug_print("%% name, value: %s, %s\n", node->name, value);
		
		switch(param[i].type){
		case P_STRING:
		{
			gchar *tmp = NULL;
			if(value && *value){
				tmp = g_locale_to_utf8(value, -1,
						       NULL, NULL, NULL);
				if(!tmp){
					g_warning("failed to convert character set.");
					tmp = g_strdup(value);
				}
			}
			g_free(*((gchar **)param[i].data));
			*((gchar **)param[i].data) = tmp;
			break;
		}
		case P_INT:
			*((gint *)param[i].data) = value ? (gint)atoi(value)
							 : 0;
			break;
		case P_BOOL:
			*((gboolean *)param[i].data) = 
				(!value || *value == '0' ||
				 *value == '\0' || !strcmp(value, "false"))
					? FALSE : TRUE;
			break;
		default:
			break;
		}
	}
}	

void prefs_write_config(PrefParam *param, const gchar *label,
			const gchar *rcfile)
{
	xmlDoc *doc;
	xmlNode *node;
	PrefFile *pfile;
	gchar *rcpath;

	g_return_if_fail(param != NULL);
	g_return_if_fail(label != NULL);
	g_return_if_fail(rcfile != NULL);

	rcpath = g_build_filename(get_rc_dir(), rcfile, NULL);
	if((pfile = prefs_file_open(rcpath)) == NULL){
		g_warning("failed to write configuration to file.\n");
		g_free(rcpath);
		return;
	}
	g_free(rcpath);

	doc = xmlNewDoc("1.0");
	doc->children = xmlNewDocNode(doc, NULL, rcfile, NULL);

	node = xmlNewChild(doc->children, NULL, label, NULL);
	prefs_node_write_param(node, param);

	xmlDocFormatDump(pfile->fp, doc, 1);
	xmlFreeDoc(doc);

	if(prefs_file_close(pfile) < 0)
		g_warning("failed to write configuration to file.\n");

	debug_print("Configuration is saved.\n");
}

#define XML_NEW_CHILD(parent, ns, name, content) \
{ \
	gchar *__tmp; \
	__tmp = strescapehtml(content); \
	xmlNewChild(parent, ns, name, __tmp); \
	g_free(__tmp); \
}

static gint prefs_node_write_param(xmlNode *node, PrefParam *param)
{
	gint i;
	gchar buf[BUFSIZE];

	for(i = 0; param[i].name; i++){
		switch(param[i].type){
		case P_STRING:
		{
			gchar *tmp = NULL;
			if(*((gchar **)param[i].data)){
				tmp = g_locale_from_utf8
					(*((gchar **)param[i].data), -1,
					 NULL, NULL, NULL);
				if(!tmp)
					tmp = g_strdup
						(*((gchar **)param[i].data));
			}
			XML_NEW_CHILD(node, NULL, param[i].name, tmp ? tmp
								     : "");
			g_free(tmp);
			break;
		}
		case P_INT:
			g_snprintf(buf, sizeof(buf), "%d",
				   *((gint *)param[i].data));
			XML_NEW_CHILD(node, NULL, param[i].name, buf);
			break;
		case P_BOOL:
			XML_NEW_CHILD(node, NULL, param[i].name, 
				      *((gboolean *)param[i].data) ? "true"
				      				    : "false");
			break;
		default:
			break;
		}
	}

	return 0;
}

#undef XML_NEW_CHILD

PrefFile *prefs_file_open(const gchar *path)
{
	PrefFile *pfile;
	gchar *tmppath;
	FILE *fp;
	gint fd;

	g_return_val_if_fail(path != NULL, NULL);

	/* open new file xxxx.tmp */
	tmppath = g_strconcat(path, ".tmp", NULL);
	if((fd = safe_creat(tmppath)) < 0){
		g_free(tmppath);
		return NULL;
	}

	if(!(fp = fdopen(fd, "wb"))){
		FILE_OP_ERROR(tmppath, "fdopen");
		close(fd);
		g_free(tmppath);
		return NULL;
	}

	pfile = g_new(PrefFile, 1);
	pfile->fp = fp;
	pfile->path = g_strdup(path);
	pfile->tmppath = tmppath;

	return pfile;
}

gint prefs_file_close(PrefFile *pfile)
{
	FILE *fp;
	gchar *path;
	gchar *tmppath;
	gchar *bakpath = NULL;

	g_return_val_if_fail(pfile != NULL, -1);

	fp = pfile->fp;
	path = pfile->path;
	tmppath = pfile->tmppath;
	g_free(pfile);

	/* close xxxx.tmp */
	if(fclose(fp) == EOF){
		FILE_OP_ERROR(tmppath, "fclose");
		unlink(tmppath);
		g_free(path);
		g_free(tmppath);
		return -1;
	}

	/* xxxx -> xxxx.bak */
	if(is_file_exist(path)){
		bakpath = g_strconcat(path, ".bak", NULL);
		if(rename(path, bakpath) < 0){
			FILE_OP_ERROR(path, "rename");
			unlink(tmppath);
			g_free(path);
			g_free(tmppath);
			g_free(bakpath);
			return -1;
		}
	}
	
	/* xxxx.tmp -> xxxx */
	if(rename(tmppath, path) < 0){
		FILE_OP_ERROR(tmppath, "rename");
		unlink(tmppath);
		g_free(path);
		g_free(tmppath);
		g_free(bakpath);
		return -1;
	}

	g_free(path);
	g_free(tmppath);
	g_free(bakpath);
	return 0;
}

gint prefs_file_close_revert(PrefFile *pfile)
{
	gchar *tmppath;

	g_return_val_if_fail(pfile != NULL, -1);

	tmppath = pfile->tmppath;
	fclose(pfile->fp);
	if(unlink(tmppath) < 0)
		FILE_OP_ERROR(tmppath, "unlink");
	g_free(tmppath);
	g_free(pfile->path);
	g_free(pfile);

	return 0;
}

void prefs_set_default(PrefParam *param)
{
	gint i;

	g_return_if_fail(param != NULL);

	debug_print("prefs_set_dafault(): start...\n");

	for(i = 0; param[i].name; i++){
		if(!param[i].data) continue;

		switch(param[i].type){
		case P_STRING:
			if(param[i].defval){
				if(param[i].defval[0] != '\0')
					*((gchar **)param[i].data) =
						g_strdup(param[i].defval);
				else
					*((gchar **)param[i].data) = NULL;
			}else
				*((gchar **)param[i].data) = NULL;
			break;
		case P_INT:
			if(param[i].defval)
				*((gint *)param[i].data) =
					(gint)atoi(param[i].defval);
			else
				*((gint *)param[i].data) = 0;
			break;
		case P_BOOL:
			if(param[i].defval){
				if(!strcasecmp(param[i].defval, "TRUE"))
					*((gboolean *)param[i].data) = TRUE;
				else
					*((gboolean *)param[i].data) =
						atoi(param[i].defval) ? TRUE
								      : FALSE;
			}else
				*((gboolean *)param[i].data) = FALSE;
			break;
		default:
			break;
		}
	}

	debug_print("prefs_set_dafault(): done.\n");
}

void prefs_free(PrefParam *param)
{
	gint i;

	g_return_if_fail(param != NULL);

	for(i = 0; param[i].name; i++){
		if(!param[i].data)
			continue;

		switch(param[i].type){
		case P_STRING:
			g_free(*((gchar **)param[i].data));
			break;
		default:
			break;
		}
	}
}

void prefs_dialog_create(PrefsDialog *dialog)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *notebook;

	g_return_if_fail(dialog != NULL);

        window = gtk_dialog_new_with_buttons
			(NULL,
			 NULL,
			 GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
			 GTK_STOCK_OK, GTK_RESPONSE_OK,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
			 NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(window), GTK_RESPONSE_OK);
	gtk_container_set_border_width(GTK_CONTAINER(window), WIN_BORDER);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	vbox = GTK_DIALOG(window)->vbox;
	gtk_box_set_spacing(GTK_BOX(vbox), VSPACING);

	notebook = gtk_notebook_new();
	gtk_widget_show(notebook);
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(notebook), 2);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);

	dialog->window		= window;
	dialog->notebook	= notebook;
}

void prefs_dialog_destroy(PrefsDialog *dialog)
{
	gtk_widget_destroy(dialog->window);
	dialog->window		= NULL;
	dialog->notebook	= NULL;
}

void prefs_button_toggled(GtkToggleButton *toggle_btn, GtkWidget *widget)
{
	gtk_widget_set_sensitive(widget,
				 gtk_toggle_button_get_active(toggle_btn));
}

void prefs_set_dialog(PrefParam *param)
{
	gint i;

	for(i = 0; param[i].name; i++){
		if(param[i].widget_set_func)
			param[i].widget_set_func(&param[i]);
	}
}

gboolean prefs_set_data_from_dialog(PrefParam *param)
{
	gboolean is_updated = FALSE;
	gint i;

	for(i = 0; param[i].name; i++){
		if(param[i].data_set_func)
			if(param[i].data_set_func(&param[i]))
				is_updated = TRUE;
	}

	return is_updated;
}

void prefs_set_dialog_to_default(PrefParam *param)
{
	gint i;
	PrefParam tmpparam;
	gchar *str_data = NULL;
	gint int_data;
	gboolean bool_data;

	for(i = 0; param[i].name; i++){
		if(!param[i].widget_set_func)
			continue;

		tmpparam = param[i];

		switch(tmpparam.type){
		case P_STRING:
			if(tmpparam.defval){

			}
			tmpparam.data = &tmpparam.defval;
			break;
		case P_INT:
			if(tmpparam.defval)
				int_data = atoi(tmpparam.defval);
			else
				int_data = 0;
			tmpparam.data = &int_data;
			break;
		case P_BOOL:
			if(tmpparam.defval){
				if(!strcasecmp(tmpparam.defval, "TRUE"))
					bool_data = TRUE;
				else
					bool_data = atoi(tmpparam.defval)
						? TRUE : FALSE;
			}else
				bool_data = FALSE;
			tmpparam.data = &bool_data;
			break;
		case P_OTHER:
			break;
		}
		tmpparam.widget_set_func(&tmpparam);
		g_free(str_data);
		str_data = NULL;
	}
}

gboolean prefs_set_data_from_entry(PrefParam *pparam)
{
	gchar **str;
	gint val;
	const gchar *entry_str;
	gboolean is_updated = FALSE;

	g_return_val_if_fail(*pparam->widget != NULL, is_updated);

	entry_str = gtk_entry_get_text(GTK_ENTRY(*pparam->widget));

	switch(pparam->type){
	case P_STRING:
		if(strcmp2(*(gchar **)pparam->data, entry_str))
			is_updated = TRUE;
		str = (gchar **)pparam->data;
		g_free(*str);
		*str = entry_str[0] ? g_strdup(entry_str)
				    : NULL;
		break;
	case P_INT:
		val = atoi(entry_str);
		if(*((gint *)pparam->data) != val)
			is_updated = TRUE;
		*((gint *)pparam->data) = val;
		break;
	default:
		g_warning("Invalid PrefType for GtkEntry widget: %d\n",
			  pparam->type);
	}

	return is_updated;
}

void prefs_set_entry_from_data(PrefParam *pparam)
{
	gchar **str;
	gchar nstr[11];

	g_return_if_fail(*pparam->widget != NULL);

	switch(pparam->type){
	case P_STRING:
		str = (gchar **)pparam->data;
		gtk_entry_set_text(GTK_ENTRY(*pparam->widget), *str ? *str
								    : "");
		break;
	case P_INT:
		g_snprintf(nstr, sizeof(nstr), "%d", *((gint *)pparam->data));
		gtk_entry_set_text(GTK_ENTRY(*pparam->widget), nstr);
		break;
	default:
		g_warning("Invalid PrefType for GtkEntry widget: %d\n",
			  pparam->type);
	}
}

gboolean prefs_set_data_from_toggle(PrefParam *pparam)
{
	gboolean is_updated = FALSE;
	gboolean val;

	g_return_val_if_fail(pparam->type == P_BOOL, is_updated);
	g_return_val_if_fail(*pparam->widget != NULL, is_updated);

	val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(*pparam->widget));
	if(*((gboolean *)pparam->data) != val)
		is_updated = TRUE;
	*((gboolean *)pparam->data) = val;

	return is_updated;
}

void prefs_set_toggle_from_data(PrefParam *pparam)
{
	g_return_if_fail(pparam->type == P_BOOL);
	g_return_if_fail(*pparam->widget != NULL);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(*pparam->widget),
				     *((gboolean *)pparam->data));
}

gint prefs_set_data_from_combo_box(PrefParam *pparam)
{
	gboolean is_updated = FALSE;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint val;

	g_return_val_if_fail(pparam->type == P_INT, 0);
	g_return_val_if_fail(*pparam->widget != NULL, 0);

	if(!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(*pparam->widget), &iter))
		val = -1;
	else{
		model = gtk_combo_box_get_model(GTK_COMBO_BOX(*pparam->widget));
		gtk_tree_model_get(model, &iter,
				   1, &val,
				   -1);
	}

	if(*((gint *)pparam->data) != val)
		is_updated = TRUE;
	*((gint *)pparam->data) = val;

	return is_updated;
}

void prefs_set_combo_box_from_data(PrefParam *pparam)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint val;

	g_return_if_fail(pparam->type == P_INT);
	g_return_if_fail(*pparam->widget != NULL);

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(*pparam->widget));

	/* empty */
	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do{
		gtk_tree_model_get(model, &iter,
				   1, &val,
				   -1);
		if(*((gint *)pparam->data) == val){
			gtk_combo_box_set_active_iter
					(GTK_COMBO_BOX(*pparam->widget), &iter);
			break;
		}
	}while(gtk_tree_model_iter_next(model, &iter));
}
