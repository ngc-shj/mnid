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
#include <libxml/tree.h>
#include <stdlib.h>

#include "pass.h"
#include "utils.h"

static gchar *xml_use_char[N_PASS_USING_CHAR] = {
	"upper",
	"lower",
	"numeric",
	"symbol",
};

GSList *pass_xmlnode_to_passlist(const xmlNode *node)
{
	GSList *passlist = NULL;
	xmlNode *cur;

	for(cur = node->children; cur; cur = cur->next){
		if(cur->type == XML_ELEMENT_NODE &&
		   !strcmp2(cur->name, "pass")){
			PassInfo *passinfo;
			passinfo = pass_read_config(cur);
		}
	}
	return passlist;
}

/* static gboolean pass_passnode_to_passlist(xmlNode *passnode) */
PassInfo *pass_read_config(xmlNode *node)
{
	PassInfo *passinfo;
	gchar *tmp;
	gint i;

	passinfo = pass_new();

	g_return_val_if_fail(node != NULL, passinfo);
	if(strcmp2(node->name, "pass"))
		return passinfo;

	passinfo->update_date =
		(tmp = xmlGetProp(node, "update_date")) ? atoi(tmp)
							: (GTime)0;

	if(node->children){
		tmp = XML_GET_CONTENT(node->children);
		if(tmp)
			passinfo->pass = g_strdup(tmp);
	}

	for(i = 0; i < N_PASS_USING_CHAR; i++)
		if(!strcmp2(xmlGetProp(node, xml_use_char[i]), "true"))
			passinfo->use_char[i] = TRUE;
		else
			passinfo->use_char[i] = FALSE;

	if(!strcmp2(xmlGetProp(node, "avoid_ambiguous"), "true"))
		passinfo->avoid_ambiguous = TRUE;
	else
		passinfo->avoid_ambiguous = FALSE;

	passinfo->digit = (tmp = xmlGetProp(node, "digit")) ? atoi(tmp)
							    : 0;

	return passinfo;
}

void pass_write_config(xmlNode *node, const PassInfo *passinfo)
{
	xmlNode *cur;
	gchar *tmp;
	gint i;
	gchar buf[BUFSIZE];

	g_return_if_fail(node != NULL);
	g_return_if_fail(passinfo != NULL);

	/* <pass>...</pass> */
	tmp = strescapehtml(passinfo->pass);
	cur = xmlNewChild(node, NULL, "pass", tmp);
	g_free(tmp);

	for(i = 0; i < N_PASS_USING_CHAR; i++){
		g_snprintf(buf, sizeof(buf), "%s",
			   passinfo->use_char[i] ? "true"
						 : "false");
		xmlSetProp(cur, xml_use_char[i], buf);
	}
	g_snprintf(buf, sizeof(buf), "%s", passinfo->avoid_ambiguous ? "true"
								     : "false");
	xmlSetProp(cur, "avoid_ambiguous", buf);

	g_snprintf(buf, sizeof(buf), "%d", passinfo->digit);
	xmlSetProp(cur, "digit", buf);

	g_snprintf(buf, sizeof(buf), "%d", passinfo->update_date);
	xmlSetProp(cur, "update_date", buf);
}

PassInfo *pass_new(void)
{
	PassInfo *passinfo;
	gint i;

	passinfo = g_new(PassInfo, 1);

	passinfo->id				= 0;
	passinfo->pass				= NULL;
	for(i = 0; i < N_PASS_USING_CHAR; i++)
		passinfo->use_char[i]		= FALSE;
	passinfo->digit				= MNID_PASS_DEFAULT;
	passinfo->avoid_ambiguous		= FALSE;
	passinfo->update_date     		= (GTime)0;

	return passinfo;
}

PassInfo *pass_copy(const PassInfo *passinfo)
{
	PassInfo *new_passinfo = pass_new();
	gint i;

	if(!passinfo)
		return new_passinfo;

	new_passinfo->id = passinfo->id;
	new_passinfo->pass = g_strdup(passinfo->pass);
	for(i = 0; i < N_PASS_USING_CHAR; i++)
		new_passinfo->use_char[i] = passinfo->use_char[i];
	new_passinfo->avoid_ambiguous = passinfo->avoid_ambiguous;
	new_passinfo->digit = passinfo->digit;
	new_passinfo->update_date = passinfo->update_date;

	return new_passinfo;
}

PassInfo *pass_copy_with_pass(const PassInfo *passinfo, const gchar *pass)
{
	PassInfo *new_passinfo = pass_copy(passinfo);
	GTimeVal current;

	if(strcmp2(new_passinfo->pass, pass)){
		g_get_current_time(&current);
		new_passinfo->update_date = current.tv_sec;
	}
	g_free(new_passinfo->pass);
	new_passinfo->pass = g_strdup(pass);

	return new_passinfo;
}

gboolean pass_update_value(PassInfo *passinfo, const gchar *pass,
		       const gboolean use_char[],
		       const gboolean avoid_ambiguous, const gint digit)
{
	gboolean is_modified = FALSE;
	GTimeVal current;
	gboolean ret_val = FALSE;
	gint i;

	if(!passinfo)
		return ret_val;

	if(strcmp2(passinfo->pass, pass)){
		g_free(passinfo->pass);
		passinfo->pass = g_strdup(pass);
		is_modified = TRUE;
		ret_val = TRUE;
	}

	for(i = 0; i < N_PASS_USING_CHAR; i++)
		if(passinfo->use_char[i] != use_char[i]){
			passinfo->use_char[i] = use_char[i];
			/* is_modified = TRUE; */
			ret_val = TRUE;
		}

	if(passinfo->avoid_ambiguous != avoid_ambiguous){
		passinfo->avoid_ambiguous = avoid_ambiguous;
		/* is_modified = TRUE; */
		ret_val = TRUE;
	}
	
	if(passinfo->digit != digit){
		passinfo->digit = digit;
		/* is_modified = TRUE; */
		ret_val = TRUE;
	}

	if(is_modified){
		g_get_current_time(&current);
		passinfo->update_date = current.tv_sec;
	}

	return ret_val;
}

void pass_free(PassInfo *passinfo)
{
	if(!passinfo) return;

	g_free(passinfo->pass);
	g_free(passinfo);
}

