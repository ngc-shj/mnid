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

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <string.h>

#include "main.h"
#include "mainwindow.h"
#include "prefs_common.h"
#include "prefs_id.h"
#include "id.h"
#include "prefs_pass.h"
#include "pass.h"
#include "pass_list.h"
#include "blowfish.h"
#include "gtkutils.h"
#include "utils.h"

static struct PrefsIdWindow {
	GtkWidget	*window;

	GtkWidget	*name_entry;
	GtkWidget	*uid_entry;
	GtkWidget	*pass_entry;
	GtkWidget	*generate_btn;
	GtkWidget	*pass_list_btn;
	GtkWidget	*uri_entry;
	GtkWidget	*uri_btn;
	GtkWidget	*remark_text;

	GtkWidget	*updated_label;

	IDinfo		*cur_idinfo;
	PassInfo	*cur_passinfo;
	IDinfo		*new_idinfo;
	PassInfo	*new_passinfo;

	PrefsIdMode	 mode;

	gboolean	 edit_finished;
} self;

static void prefs_id_create		(void);
static void prefs_id_ok			(void);
static void prefs_id_cancel		(void);

static void prefs_id_idinfo_to_dialog	(void);
static IDinfo *prefs_id_dialog_to_idinfo(void);

/* callback functions */
static gint dialog_key_press_event_cb	(GtkWidget	*widget,
					 GdkEventKey	*event,
					 gpointer	 data);
static void dialog_response_cb		(GtkDialog	*dialog,
					 gint		 response_id,
					 gpointer	 data);
static void generate_password_clicked_cb(void);
static void password_list_clicked_cb	(void);
static void jump_to_uri_clicked_cb	(void);

IDinfo *prefs_id_open(IDinfo *idinfo, PrefsIdMode mode)
{
	IDinfo *new_idinfo;

	if(!self.window)
		prefs_id_create();


	self.cur_idinfo = idinfo;
	self.cur_passinfo = idinfo ? idinfo->passinfo
				   : NULL;
	self.mode = mode;
	
	prefs_id_idinfo_to_dialog();

	self.new_idinfo = NULL;
	self.new_passinfo = NULL;
	self.edit_finished = FALSE;

	while(self.edit_finished == FALSE){
		gtk_dialog_run(GTK_DIALOG(self.window));
	}
	gtk_widget_hide(self.window);

	new_idinfo = self.new_idinfo;
	self.cur_passinfo = NULL;
	self.cur_idinfo   = NULL;
	self.new_passinfo = NULL;
	self.new_idinfo   = NULL;

	if(new_idinfo)
		debug_print("new idinfo created. %s\n", new_idinfo->name);

	return new_idinfo;
}

static void prefs_id_create(void)
{
	GtkWidget *dialog;
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *name_entry;
	GtkWidget *uid_entry;
	GtkWidget *pass_entry;
	GtkWidget *generate_btn;
	GtkWidget *pass_list_btn;
	GtkWidget *uri_entry;
	GtkWidget *uri_btn;
	GtkWidget *scrolledwin;
	GtkWidget *remark_text;

	/* dialog */
	dialog = gtk_dialog_new_with_buttons
			(NULL,
			 NULL,
			 GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
			 GTK_STOCK_OK, GTK_RESPONSE_OK,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), WIN_BORDER);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_widget_set_size_request(dialog, 480, 360);
	gtk_widget_realize(dialog);

	vbox = GTK_DIALOG(dialog)->vbox;

	g_signal_connect(G_OBJECT(dialog), "key_press_event",
			 G_CALLBACK(dialog_key_press_event_cb), NULL);
	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(dialog_response_cb), NULL);

	/* table */
	table = gtk_table_new(6, 4, FALSE);
	gtk_widget_show(table);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 8);

#define TABLE_SET_ATTACH_LABEL(name, left, right, top, bottom, entry) \
{ \
	label = gtk_label_new_with_mnemonic(name); \
	gtk_widget_show(label); \
	gtk_table_attach(GTK_TABLE(table), label, \
			 left, right, top, bottom, \
			 GTK_FILL, 0, 0, 0); \
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT); \
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), entry); \
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5); \
}

	/* name */
	name_entry = gtk_entry_new();
	gtk_widget_show(name_entry);
	gtk_table_attach(GTK_TABLE(table), name_entry,
			 1, 4, 0, 1,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);
	TABLE_SET_ATTACH_LABEL(_("_Name"), 0, 1, 0, 1, name_entry);

	/* id */
	uid_entry = gtk_entry_new();
	gtk_widget_show(uid_entry);
	gtk_table_attach(GTK_TABLE(table), uid_entry,
			 1, 4, 1, 2,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);
	TABLE_SET_ATTACH_LABEL(_("User _ID"), 0, 1, 1, 2, uid_entry);

	/* pass */
	pass_entry = gtk_entry_new();
	gtk_widget_show(pass_entry);
	gtk_table_attach(GTK_TABLE(table), pass_entry,
			 1, 2, 2, 3,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);
	/* gtk_editable_set_editable(GTK_EDITABLE(pass_entry), FALSE); */
	TABLE_SET_ATTACH_LABEL(_("_Password"), 0, 1, 2, 3, pass_entry);

	generate_btn = gtk_button_new_with_mnemonic(_("_Generate..."));
	gtk_widget_show(generate_btn);
	gtk_table_attach(GTK_TABLE(table), generate_btn,
			 2, 3, 2, 3,
			 GTK_FILL, 0, 0, 0);
	g_signal_connect(G_OBJECT(generate_btn), "clicked",
			 G_CALLBACK(generate_password_clicked_cb), NULL);
	
	pass_list_btn = gtk_button_new_with_mnemonic(_("_History..."));
	gtk_widget_show(pass_list_btn);
	gtk_table_attach(GTK_TABLE(table), pass_list_btn,
			 3, 4, 2, 3,
			 GTK_FILL, 0, 0, 0);
	g_signal_connect(G_OBJECT(pass_list_btn), "clicked",
			 G_CALLBACK(password_list_clicked_cb), NULL);
	
	/* timestamp */
	label = gtk_label_new(NULL);
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label,
			 1, 3, 3, 4,
			 GTK_FILL, 0, 0, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtkutils_widget_set_font(label, -1, -1, PANGO_SCALE_SMALL);
	self.updated_label = label;

	/* uri */
	uri_entry = gtk_entry_new();
	gtk_widget_show(uri_entry);
	gtk_table_attach(GTK_TABLE(table), uri_entry,
			 1, 3, 4, 5,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);
	TABLE_SET_ATTACH_LABEL(_("_URI"), 0, 1, 4, 5, uri_entry);

	uri_btn = gtk_button_new_from_stock(GTK_STOCK_JUMP_TO);
	gtk_widget_show(uri_btn);
	gtk_table_attach(GTK_TABLE(table), uri_btn,
			 3, 4, 4, 5,
			 GTK_FILL, 0, 0, 0);
	g_signal_connect(G_OBJECT(uri_btn), "clicked",
			 G_CALLBACK(jump_to_uri_clicked_cb), NULL);
	
	/* remark */
	scrolledwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrolledwin);
	gtk_table_attach(GTK_TABLE(table), scrolledwin,
			 1, 4, 5, 6,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
					    GTK_SHADOW_IN);

	remark_text = gtk_text_view_new();
	gtk_widget_show(remark_text);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(remark_text), GTK_WRAP_WORD);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(remark_text), TRUE);
	gtk_container_add(GTK_CONTAINER(scrolledwin), remark_text);
	gtk_widget_set_size_request(remark_text, -1, 140);
	TABLE_SET_ATTACH_LABEL(_("_Remarks"), 0, 1, 5, 6, remark_text);

	/* pointer */
	self.window		= dialog;
	self.name_entry		= name_entry;
	self.uid_entry		= uid_entry;
	self.pass_entry		= pass_entry;
	self.generate_btn	= generate_btn;
	self.pass_list_btn	= pass_list_btn;
	self.uri_entry		= uri_entry;
	self.uri_btn		= uri_btn;
	self.remark_text	= remark_text;
}

static void prefs_id_idinfo_to_dialog(void)
{
	IDinfo *idinfo = self.cur_idinfo;
	PassInfo *passinfo;
	GtkTextView *textview = GTK_TEXT_VIEW(self.remark_text);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(textview);
	GtkTextIter iter;
	static gint count = 1;
	gchar idinfo_name[32];
	gchar *title;
	gchar *mode;

	switch(self.mode){
	case PREFS_ID_MODE_NEW:
		mode = _("New ID infomation");
		break;
	case PREFS_ID_MODE_EDIT:
		mode = _("Edit ID information");
		break;
	case PREFS_ID_MODE_COPY:
		mode = _("Copy ID information");
		break;
	default:
		mode = "";
		break;
	}
	title = g_strdup_printf("%s - %s", mode, FORMAL_NAME);
	gtk_window_set_title(GTK_WINDOW(self.window), title);
	g_free(title);

	if(!idinfo){
		g_snprintf(idinfo_name, sizeof(idinfo_name), "ID %d", count++);
		gtk_entry_set_text(GTK_ENTRY(self.name_entry), idinfo_name);
		gtk_entry_set_text(GTK_ENTRY(self.uid_entry), "");
		gtk_entry_set_text(GTK_ENTRY(self.pass_entry), "");
		gtk_label_set_label(GTK_LABEL(self.updated_label), "");
		gtk_entry_set_text(GTK_ENTRY(self.uri_entry), "");

		gtk_text_buffer_set_text(buffer, "", -1);
		gtk_text_buffer_get_start_iter(buffer, &iter);
		gtk_text_buffer_insert(buffer, &iter, "", -1);
		return;
	}
	
	/* Name */
	gtk_entry_set_text(GTK_ENTRY(self.name_entry),
			   idinfo->name ? idinfo->name
			   		: ""); 
	/* User ID */
	gtk_entry_set_text(GTK_ENTRY(self.uid_entry),
			   idinfo->uid ? idinfo->uid
			   	       : "");
	/* Password Info */
	passinfo = idinfo->passinfo;
	if(passinfo){
		GTime update_date = passinfo->update_date;

		/* Password */
		gtk_entry_set_text(GTK_ENTRY(self.pass_entry),
				   passinfo->pass ? passinfo->pass
			   			  : "");
		/* Updated %s ago */
		if(update_date){
			GTimeVal current;
			gchar *buf;
			gchar *p;

			g_get_current_time(&current);
			p = elapsed_time(update_date, current.tv_sec);
			buf = g_strdup_printf(_("Updated %s ago"), p);
			gtk_label_set_label(GTK_LABEL(self.updated_label), buf);
			g_free(p);
			g_free(buf);
		}else
			gtk_label_set_label(GTK_LABEL(self.updated_label), "");
	}else
		gtk_entry_set_text(GTK_ENTRY(self.pass_entry), "");

	/* URI */
	gtk_entry_set_text(GTK_ENTRY(self.uri_entry),
			   idinfo ? idinfo->uri ? idinfo->uri
			   			: ""
			   	  : "");

	/* Remarks */
	gtk_text_buffer_set_text(buffer, "\0", -1);
	gtk_text_buffer_get_start_iter(buffer, &iter);
	gtk_text_buffer_insert(buffer, &iter,
			       idinfo->remark ? idinfo->remark
			       		      : "",
			       -1);
}

static IDinfo *prefs_id_dialog_to_idinfo(void)
{
	IDinfo *idinfo;
	const gchar *name = NULL;
	const gchar *uid = NULL;
	const gchar *pass = NULL;
	const gchar *uri = NULL;
	const gchar *remark = NULL;
	GtkTextView *textview;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	gchar *error_msg = NULL;

	name = gtk_entry_get_text(GTK_ENTRY(self.name_entry));
	if(!name || *name == '\0'){
		error_msg = _("Name is not specified.");
		goto error;
	}
	uid = gtk_entry_get_text(GTK_ENTRY(self.uid_entry));
	if(!uid || *uid == '\0'){
		error_msg = _("User ID is not specified.");
		goto error;
	}
	pass = gtk_entry_get_text(GTK_ENTRY(self.pass_entry));
	uri = gtk_entry_get_text(GTK_ENTRY(self.uri_entry));

	textview = GTK_TEXT_VIEW(self.remark_text);
	buffer = gtk_text_view_get_buffer(textview);
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_iter_at_offset(buffer, &end, -1);
	remark = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

error:
	if(error_msg){
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new(NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_OK,
						"%s", error_msg);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return NULL;
	}

	/* id info. */
	/* edit or copy */
	if(self.cur_idinfo)
		idinfo = id_copy(self.cur_idinfo);
	/* add */
	else
		idinfo = id_new();
	id_update_value(idinfo, name, uid, uri, remark);

	/* pass info. */
	if(self.new_passinfo){
		idinfo->passinfo = pass_copy_with_pass(self.new_passinfo, pass);
		pass_free(self.new_passinfo);
	}else{
		idinfo->pass_list = g_slist_remove(idinfo->pass_list,
						   idinfo->passinfo);
		idinfo->passinfo = pass_copy_with_pass(self.cur_passinfo, pass);
	}	
	idinfo->pass_list = g_slist_append(idinfo->pass_list, idinfo->passinfo);

	return idinfo;
}

/* callback functions */

static gint dialog_key_press_event_cb(GtkWidget *widget, GdkEventKey *event,
					gpointer data)
{
	if(event && event->keyval == GDK_Escape){
		gtk_dialog_response(GTK_DIALOG(self.window),
				    GTK_RESPONSE_CANCEL);
		return TRUE;
	}

	return FALSE;
}

static void dialog_response_cb(GtkDialog *dialog, gint response_id,
				gpointer data)
{
	switch(response_id)
	{
	case GTK_RESPONSE_OK:
		prefs_id_ok();
		break;
	case GTK_RESPONSE_CANCEL:
		prefs_id_cancel();
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		gtk_dialog_response(GTK_DIALOG(self.window),
				    GTK_RESPONSE_CANCEL);
		break;
	case GTK_RESPONSE_NONE:
		break;
	default:
		break;
	}
}

static void prefs_id_ok(void)
{
	IDinfo *idinfo;

	idinfo = prefs_id_dialog_to_idinfo();
	if(idinfo){
		self.new_idinfo = idinfo;
		self.edit_finished = TRUE;
	}
}

static void prefs_id_cancel(void)
{
	if(self.new_passinfo)
		pass_free(self.new_passinfo);

	self.new_passinfo = NULL;
	self.new_idinfo = NULL;
	self.edit_finished = TRUE;
}

static void generate_password_clicked_cb(void)
{
	PassInfo *passinfo, *new_passinfo;

	if(self.new_passinfo)
		passinfo = self.new_passinfo;
	else
		passinfo = self.cur_passinfo;

	new_passinfo = prefs_pass_open(passinfo);

	if(new_passinfo){
		gtk_entry_set_text(GTK_ENTRY(self.pass_entry),
				   new_passinfo->pass);

		pass_free(self.new_passinfo);
		self.new_passinfo = new_passinfo;
	}
}

static void password_list_clicked_cb(void)
{
	if(!self.cur_idinfo)
		return;

	if(!self.cur_idinfo->pass_list)
		return;

	pass_list_window_open(self.cur_idinfo->pass_list);
}

static void jump_to_uri_clicked_cb(void)
{
	const gchar *uri;

        uri = gtk_entry_get_text(GTK_ENTRY(self.uri_entry));
	if(!uri || *uri == '\0')
		return;

	open_uri(uri, prefs_common.uri_cmd);
}

gboolean prefs_id_read_config(void)
{
	IDinfo *idinfo;
	EncryptType encrypt_type = ENCRYPT_TYPE_NONE;
	gpointer data = NULL;

	debug_print("Reading ID configuration...\n");

	/* remove all previous id list */
	while(prefs_common.idlist != NULL){
		idinfo = (IDinfo *)prefs_common.idlist->data;
		id_free(idinfo);
		prefs_common.idlist = g_slist_remove(prefs_common.idlist,
						     idinfo);
	}

        /* decryption */
	if(prefs_common.use_encrypt){
		switch (prefs_common.encrypt_type){
		case ENCRYPT_TYPE_BLOWFISH:
			/* data = &prefs_common.bf_key; */
			data = NULL;
			encrypt_type = ENCRYPT_TYPE_BLOWFISH;
			break;
#if USE_GPGME
		case ENCRYPT_TYPE_GNUPG:
			if(prefs_common.keyid && *prefs_common.keyid != '\0'){
				encrypt_type = ENCRYPT_TYPE_GNUPG;
				data = prefs_common.keyid;
			}
			break;
#endif /* USE_GPGME */
		default:
			break;
		}
	}

	if(!id_read_config(&prefs_common.idlist, encrypt_type, data))
		return FALSE;

	debug_print("Reading ID configureation...Done.\n");

	return TRUE;
}

void prefs_id_write_config(void)
{
	EncryptType encrypt_type = ENCRYPT_TYPE_NONE;
	gpointer data = NULL;

        /* encryption */
	if(prefs_common.use_encrypt){
		switch (prefs_common.encrypt_type){
		case ENCRYPT_TYPE_BLOWFISH:
			if(blowfish_key_get())
				encrypt_type = ENCRYPT_TYPE_BLOWFISH;
			break;
#if USE_GPGME
		case ENCRYPT_TYPE_GNUPG:
			if(prefs_common.keyid && *prefs_common.keyid != '\0'){
				encrypt_type = ENCRYPT_TYPE_GNUPG;
				data = prefs_common.keyid;
			}
			break;
#endif /* USE_GPGME */
		default:
			break;
		}
	}

	id_write_config(prefs_common.idlist, encrypt_type, data);
	data = NULL;
}
