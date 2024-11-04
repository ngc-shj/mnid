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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <errno.h>

#include "prefs.h"
#include "id.h"
#include "pass.h"
#if USE_GPGME
#  include "gnupg_crypt.h"
#endif /* USE_GPGME */
#include "blowfish_crypt.h"
#include "utils.h"
#include "version.h"

static GSList *id_xmldoc_to_idlist	(xmlDoc		*doc);
static gboolean id_idnode_to_idlist	(xmlNode	*idnode,
					 GSList	       **idlist);
static void id_set_value		(IDinfo		*idinfo,
					 const gchar	*name,
					 const gchar	*uid,
					 const gchar	*uri,
					 const gchar	*remark,
					 const GTime	 create_date,
					 const GTime	 update_date);

gboolean id_read_config(GSList **idlist, EncryptType encrypt_type,
			gpointer data)
{
	FILE *fp = NULL;
	xmlDoc *doc = NULL;
	gchar *rcpath = NULL;
	gchar *f_data = NULL;
	size_t f_size;
	gboolean ret_val = FALSE;

	*idlist = NULL;

	rcpath = g_build_filename(get_rc_dir(), ID_LIST, NULL);
	if(!(fp = safe_fopen(rcpath))){
		g_free(rcpath);
		return TRUE;
	}
	g_free(rcpath);

	debug_print("%% encrypt_type : %d\n", encrypt_type);

	/* decryption */
	switch(encrypt_type){
	case ENCRYPT_TYPE_BLOWFISH:
		ret_val = blowfish_decrypt(fp, &f_data, &f_size);
		break;
#if USE_GPGME
	case ENCRYPT_TYPE_GNUPG:
		ret_val = gnupg_decrypt(fp, &f_data, &f_size);
		break;
#endif /* USE_GPGME */
	default:
		ret_val = file_get_contents(fp, &f_data, &f_size);
		break;
	}
	fclose(fp);

	if(ret_val){
		doc = xmlReadMemory(f_data, f_size, NULL, NULL, 0);
		g_free(f_data);
	}else{
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new(NULL,
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					_("Decryption failed.\n"
				   	  "Exit."));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return FALSE;
	}

	if(!doc){
		g_warning("Can't parse.\n");
		return TRUE;
	}

	*idlist = id_xmldoc_to_idlist(doc);

	xmlFreeDoc(doc);

	return TRUE;
}

static gboolean id_idnode_to_idlist(xmlNode *idnode, GSList **idlist)
{
	xmlNode *cur;
	IDinfo *idinfo;
	GSList *pass_list = NULL;
	PassInfo *passinfo = NULL;
	GTime create_date;
	GTime update_date;
	gchar *name = NULL;
	gchar *uid = NULL;
	gchar *uri = NULL;
	gchar *remark = NULL;
	gchar *p;
	
	g_return_val_if_fail(idnode != NULL, FALSE);

	create_date = (p = xmlGetProp(idnode, "create_date")) ? atoi(p)
							      : (GTime)0;
	update_date = (p = xmlGetProp(idnode, "update_date")) ? atoi(p)
							      : (GTime)0;

	for(cur = idnode->children; cur; cur= cur->next){
		if(cur->type == XML_ELEMENT_NODE){
			xmlNode *child = cur->children;

			if(!strcmp2(cur->name, "name")){
				if(child && child->type == XML_TEXT_NODE)
					name = XML_GET_CONTENT(child);
			}
			if(!strcmp2(cur->name, "uid")){
				if(child && child->type == XML_TEXT_NODE)
					uid = XML_GET_CONTENT(child);
			}
			if(!strcmp2(cur->name, "pass")){
				passinfo = pass_read_config(cur);
				pass_list = g_slist_append(pass_list, passinfo);
			}
			if(!strcmp2(cur->name, "uri")){
				if(child && child->type == XML_TEXT_NODE)
					uri = XML_GET_CONTENT(child);
			}
			if(!strcmp2(cur->name, "remark")){
				if(child && child->type == XML_TEXT_NODE)
					remark = XML_GET_CONTENT(child);
			}
		}
	}

	idinfo = id_new();
	id_set_value(idinfo, name, uid, uri, remark, create_date, update_date);
	if(idinfo){
		idinfo->passinfo = passinfo;
		idinfo->pass_list = pass_list;
	}
	*idlist = g_slist_append(*idlist, idinfo);

	return TRUE;
}

static GSList *id_xmldoc_to_idlist(xmlDoc *doc)
{
	GSList *idlist = NULL;
	xmlNode *cur;

	g_return_val_if_fail(doc != NULL, NULL);

	cur = xmlDocGetRootElement(doc);

	for(cur = cur->children; cur; cur = cur->next){
		if(cur->type == XML_ELEMENT_NODE && !strcmp2(cur->name, "id"))
			if(!id_idnode_to_idlist(cur, &idlist))
				return NULL;
	}

	return idlist;
}

#define XML_NEW_CHILD(parent, ns, name, content) \
{ \
	gchar *__tmp; \
	__tmp = strescapehtml(content); \
	xmlNewChild(parent, ns, name, __tmp); \
	g_free(__tmp); \
}

void id_write_config(GSList *idlist, EncryptType encrypt_type, gpointer data)
{
	xmlDoc *doc;
	xmlNode *tree;
	xmlChar *xml_mem = NULL;
	size_t xml_size;
	gchar *rcpath;
	PrefFile *pfile;
	GSList *cur;
	gchar buf[BUFSIZE];
	gboolean ret_val;

	rcpath = g_build_filename(get_rc_dir(), ID_LIST, NULL);
	if((pfile = prefs_file_open(rcpath)) == NULL){
		g_warning("failed to write id configuration to file.\n");
		g_free(rcpath);
		return;
	}
	g_free(rcpath);

	doc = xmlNewDoc("1.0");
	doc->children = xmlNewDocNode(doc, NULL, "idlist", NULL);
	for(cur = idlist; cur; cur = cur->next){
		IDinfo *idinfo = (IDinfo *)cur->data;
		GSList *pass;

		tree = xmlNewChild(doc->children, NULL, "id", NULL);
		g_snprintf(buf, sizeof(buf), "%d", idinfo->create_date);
		xmlSetProp(tree, "create_date", buf);
		g_snprintf(buf, sizeof(buf), "%d", idinfo->update_date);
		xmlSetProp(tree, "update_date", buf);

		XML_NEW_CHILD(tree, NULL, "name", idinfo->name);
		XML_NEW_CHILD(tree, NULL, "uid", idinfo->uid);
		for(pass = idinfo->pass_list; pass; pass = pass->next)
			pass_write_config(tree, (PassInfo *)pass->data);
		XML_NEW_CHILD(tree, NULL, "uri", idinfo->uri);
		XML_NEW_CHILD(tree, NULL, "remark", idinfo->remark);
	}
	
	/* encryption */
	switch(encrypt_type){
	case ENCRYPT_TYPE_BLOWFISH:
		xmlDocDumpFormatMemory(doc, &xml_mem, &xml_size, 1);
		ret_val = blowfish_encrypt(pfile->fp, xml_mem, xml_size);
		break;
#if USE_GPGME
	case ENCRYPT_TYPE_GNUPG:
		xmlDocDumpFormatMemory(doc, &xml_mem, &xml_size, 1);
		ret_val = gnupg_encrypt(pfile->fp, xml_mem, xml_size,
					(gchar *)data);
		break;
#endif /* USE_GPGME */
	default:
		xmlDocFormatDump(pfile->fp, doc, 1);
		ret_val = TRUE;
		break;
	}

	if(!ret_val){
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new(NULL,
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					_("Encryption failed.\n"
					  "Save configuration as plain text!"));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		xmlDocFormatDump(pfile->fp, doc, 1);
	}

	if(xml_mem)
		xmlFree(xml_mem);
	xmlFreeDoc(doc);

	if(prefs_file_close(pfile) < 0){
		g_warning("failed to write configuration to file.\n");
		return;
	}
}

#undef XML_NEW_CHILD

IDinfo *id_new(void)
{
	IDinfo *idinfo;
	
	idinfo = g_new0(IDinfo, 1);

	idinfo->name		= NULL;
	idinfo->uid		= NULL;
	idinfo->uri		= NULL;
	idinfo->remark		= NULL;
	idinfo->passinfo	= NULL;
	idinfo->pass_list	= NULL;

	idinfo->create_date	= (GTime)0;
	idinfo->update_date	= (GTime)0;

	return idinfo;
}

#define SET_ID_VALUE(dest, src) \
{ \
	if(strcmp2(dest, src)){ \
		g_free(dest); \
		dest = g_strdup(src); \
	} \
}

static void id_set_value(IDinfo *idinfo, const gchar *name, const gchar *uid,
	       const gchar *uri, const gchar *remark,
	       const GTime create_date, const GTime update_date)
{
	if(!idinfo) return;

	SET_ID_VALUE(idinfo->name, name);
	SET_ID_VALUE(idinfo->uid, uid);
	SET_ID_VALUE(idinfo->uri, uri);
	SET_ID_VALUE(idinfo->remark, remark);

	if(create_date > -1)
		idinfo->create_date = create_date;
	if(update_date > -1)
		idinfo->update_date = update_date;
}

#undef SET_ID_VALUE

#define UPDATE_ID_VALUE(dest, src) \
{ \
	if(strcmp2(dest, src)){ \
		g_free(dest); \
		dest = g_strdup(src); \
		is_modified = TRUE; \
	} \
}

void id_update_value(IDinfo *idinfo, const gchar *name, const gchar *uid,
	       const gchar *uri, const gchar *remark)
{
	gboolean is_modified = FALSE;
	GTimeVal current;

	if(!idinfo) return;

	UPDATE_ID_VALUE(idinfo->name, name);
	UPDATE_ID_VALUE(idinfo->uid, uid);
	UPDATE_ID_VALUE(idinfo->uri, uri);
	UPDATE_ID_VALUE(idinfo->remark, remark);

	if(is_modified){
		g_get_current_time(&current);

		if(!idinfo->create_date)
			idinfo->create_date = current.tv_sec;
		idinfo->update_date = current.tv_sec;
	}
}

#undef UPDATE_ID_VALUE

IDinfo *id_copy(const IDinfo *idinfo)
{
	IDinfo *new_idinfo = id_new();
	GSList *pass;

	if(!idinfo) return new_idinfo;

	new_idinfo->name	= g_strdup(idinfo->name);
	new_idinfo->uid		= g_strdup(idinfo->uid);
	new_idinfo->uri		= g_strdup(idinfo->uri);
	new_idinfo->remark	= g_strdup(idinfo->remark);
	/* new_idinfo->passinfo	= pass_copy(idinfo->passinfo); */
	/* new_idinfo->pass_list	= NULL; */
	for(pass = idinfo->pass_list; pass; pass = pass->next){
		PassInfo *new_passinfo;

		new_passinfo = pass_copy((PassInfo *)pass->data);
		new_idinfo->pass_list = g_slist_append(new_idinfo->pass_list,
						       new_passinfo);
		new_idinfo->passinfo = new_passinfo;
	}

	new_idinfo->create_date	= idinfo->create_date;
	new_idinfo->update_date	= idinfo->update_date;
	
	return new_idinfo;
}

void id_free(IDinfo *idinfo)
{
	GSList *pass;

	if(!idinfo) return;

	/* pass_free(idinfo->passinfo); */
	for(pass = idinfo->pass_list; pass; pass = pass->next)
		pass_free((PassInfo *)pass->data);
	g_slist_free(idinfo->pass_list);

	g_free(idinfo->name);
	g_free(idinfo->uid);
	g_free(idinfo->uri);
	g_free(idinfo->remark);

	g_free(idinfo);
}
