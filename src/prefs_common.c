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
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <string.h>
#define __USE_XOPEN
#include <unistd.h>

#include "mainwindow.h"
#include "prefs.h"
#include "prefs_common.h"
#include "prefs_id.h"
#if USE_GPGME
#  include "gnupg_select_key.h"
#endif /* USE_GPGME */
#include "blowfish_input_key.h"
#include "blowfish.h"
#include "utils.h"
#include "gtkutils.h"

PrefsCommon prefs_common;

static PrefsDialog prefs_dialog;

static struct Privacy{
	GtkWidget *encrypt_type_combo;
	GtkWidget *use_encrypt_chkbtn;
#if USE_GPGME
	GtkWidget *gpg_keyid_btn;
	GtkWidget *keyid_entry;
#endif /* USE_GPGME */
	GtkWidget *bf_key_btn;
	GtkWidget *use_encrypt_label;
	gchar *key;
} privacy;

static struct Other {
	GtkWidget *uri_entry;
} other;

static PrefParam param[] = {
	/* Widget size */
	{"mainwin_x", "128", &prefs_common.mainwin_x, P_INT, NULL, NULL, NULL},
	{"mainwin_y", "128", &prefs_common.mainwin_y, P_INT, NULL, NULL, NULL},

	{"toolbar_style", "3", &prefs_common.toolbar_style, P_INT,
		NULL, NULL, NULL},
	{"show_statusbar", "TRUE", &prefs_common.show_statusbar, P_BOOL,
		NULL, NULL, NULL},

	/* Privacy */
	{"use_encrypt", "FALSE", &prefs_common.use_encrypt, P_BOOL,
		&privacy.use_encrypt_chkbtn,
		prefs_set_data_from_toggle, prefs_set_toggle_from_data},
	{"encrypt_type", "0", &prefs_common.encrypt_type, P_INT,
		&privacy.encrypt_type_combo,
		prefs_set_data_from_combo_box, prefs_set_combo_box_from_data},

#if USE_GPGME
	{"keyid", "", &prefs_common.keyid, P_STRING,
		&privacy.keyid_entry,
		prefs_set_data_from_entry, prefs_set_entry_from_data},
#endif /* USE_GPGME */

	/* Other */
	{"uri_open_command", DEFAULT_BROWSER_CMD,
		&prefs_common.uri_cmd, P_STRING,
		&other.uri_entry,
		prefs_set_data_from_entry, prefs_set_entry_from_data},

	{NULL, NULL, NULL, P_OTHER, NULL, NULL, NULL},
};

/* widget creating functions */
static void prefs_common_create			(void);
static void prefs_common_privacy_create		(void);
static void prefs_common_other_create		(void);

static void prefs_common_ok			(void);
static gboolean prefs_common_apply		(void);
static void prefs_common_cancel			(void);

/* privacy */
static GtkWidget *prefs_common_encrypt_type_create
						(void);
static void encrypt_type_changed_cb		(GtkComboBox	*combo_box,
						 gpointer	 data);
static void use_encrypt_toggled_cb		(GtkWidget	*widget,
						 gpointer	data);
static void prefs_common_encrypt_toggled	(void);

/* callback functions */
static gboolean dialog_key_press_event_cb	(GtkWidget	*widget,
						 GdkEventKey	*event,
						 gpointer	 data);
static void dialog_response_cb			(GtkDialog	*dialog,
						 gint		 response_id,
						 gpointer	 data);
/* Privacy */
#if USE_GPGME
static void gpg_keyid_clicked_cb		(void);
#endif /* USE_GPGME */
static void blowfish_key_clicked_cb		(void);

/* */

void prefs_common_read_config(void)
{
	prefs_read_config(param, "common", COMMON_RC);
}

void prefs_common_write_config(void)
{
	prefs_write_config(param, "common", COMMON_RC);
}

void prefs_common_open(MainWindow *mainwin)
{
	if(!prefs_dialog.window)
		prefs_common_create();

	prefs_dialog.parent = mainwin;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(prefs_dialog.notebook), 0);
	gtk_dialog_set_default_response(GTK_DIALOG(prefs_dialog.window),
					GTK_RESPONSE_OK);

	prefs_set_dialog(param);

	prefs_dialog.is_finished = FALSE;
	while(prefs_dialog.is_finished == FALSE)
		gtk_dialog_run(GTK_DIALOG(prefs_dialog.window));

	gtk_widget_hide(prefs_dialog.window);
}

static void prefs_common_create(void)
{
	GtkWidget *label;
	gchar *buf;
	gint page = 0;

	debug_print("Creating common preferences window...\n");

	prefs_dialog_create(&prefs_dialog);
	buf = g_strdup_printf("%s - %s", _("Common Preferences"), FORMAL_NAME);
	gtk_window_set_title(GTK_WINDOW(prefs_dialog.window), buf);
	g_free(buf);

	g_signal_connect(G_OBJECT(prefs_dialog.window), "key_press_event",
			 G_CALLBACK(dialog_key_press_event_cb), NULL);
	g_signal_connect(G_OBJECT(prefs_dialog.window), "response",
			 G_CALLBACK(dialog_response_cb), NULL);

	/* create all widgets on notebook */
	/* Privacy */
	prefs_common_privacy_create();
	label = gtk_label_new(_("Security"));
	gtk_widget_show(label);
	gtk_notebook_set_tab_label
		(GTK_NOTEBOOK(prefs_dialog.notebook),
		 gtk_notebook_get_nth_page
		 		(GTK_NOTEBOOK(prefs_dialog.notebook), page++),
		 		 label);

	/* Other */
	prefs_common_other_create();
	label = gtk_label_new(_("Other"));
	gtk_widget_show(label);
	gtk_notebook_set_tab_label
		(GTK_NOTEBOOK(prefs_dialog.notebook),
		 gtk_notebook_get_nth_page
		 		(GTK_NOTEBOOK(prefs_dialog.notebook), page++),
				 label);
}

static gboolean dialog_key_press_event_cb(GtkWidget *widget,
					GdkEventKey *event, gpointer data)
{
	if(event && event->keyval == GDK_Escape){
		gtk_dialog_response(GTK_DIALOG(prefs_dialog.window),
				    GTK_RESPONSE_CANCEL);
		return TRUE;
	}
	return FALSE;
}

static void dialog_response_cb(GtkDialog *dialog, gint response_id,
				  gpointer  data)
{
	switch(response_id)
	{
	case GTK_RESPONSE_OK:
		prefs_common_ok();
		break;
	case GTK_RESPONSE_CANCEL:
		prefs_common_cancel();
		break;
	case GTK_RESPONSE_APPLY:
		prefs_common_apply();
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		gtk_dialog_response(GTK_DIALOG(prefs_dialog.window),
				    GTK_RESPONSE_CANCEL);
		break;
	case GTK_RESPONSE_NONE:
		break;
	default:
		break;
	}
}

static void prefs_common_ok(void)
{
	if(prefs_common_apply())
		prefs_dialog.is_finished = TRUE;
}

static gboolean prefs_common_apply(void)
{
	GtkComboBox *combo_box = GTK_COMBO_BOX(privacy.encrypt_type_combo);
	GtkToggleButton *toggle = GTK_TOGGLE_BUTTON(privacy.use_encrypt_chkbtn);
	GtkTreeIter iter;
	GtkTreeModel *model;
	gboolean use_encrypt = prefs_common.use_encrypt;
	EncryptType encrypt_type = prefs_common.encrypt_type;
	gboolean write_idinfo = FALSE;
	const gchar *key = NULL;
#ifdef USE_GPGME
	const gchar *keyid = NULL;
#endif /* USE_GPGME */


	/* cross edit */
	use_encrypt = gtk_toggle_button_get_active(toggle);

	if(gtk_combo_box_get_active_iter(combo_box, &iter)){
		model = gtk_combo_box_get_model(combo_box);
		gtk_tree_model_get(model, &iter,
				   1, &encrypt_type,
				   -1);
	}

	if(use_encrypt){
		switch(encrypt_type){
		case ENCRYPT_TYPE_BLOWFISH:
			key = blowfish_key_get();

			/* key is null... */
			if((!privacy.key || *privacy.key == '\0') &&
			   (!key || *key == '\0')){
				GtkWidget *dialog;

				dialog = gtk_message_dialog_new(NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_OK,
						_("Bear blowfish key."));
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
				key = NULL;
				return FALSE;
			}
			key = NULL;
			break;
#ifdef USE_GPGME
		case ENCRYPT_TYPE_GNUPG:
			keyid = gtk_entry_get_text
					(GTK_ENTRY(privacy.keyid_entry));
			if((!prefs_common.keyid || *prefs_common.keyid == '\0') &&
			   (!keyid || *keyid == '\0')){
				GtkWidget *dialog;

				dialog = gtk_message_dialog_new(NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_OK,
						_("Select secret key."));
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
				return FALSE;
			}
			break;
#endif /* USE_GPGME */
		default:
			break;
		}
	}


	/* encryption configuration is updated... */
	if(prefs_common.use_encrypt != use_encrypt ||
	   (use_encrypt && prefs_common.encrypt_type != encrypt_type) ||
#ifdef USE_GPGME
	   (use_encrypt && encrypt_type == ENCRYPT_TYPE_GNUPG &&
	    strcmp2(prefs_common.keyid, keyid)) ||
#endif /* USE_GPGME */
	   (use_encrypt && encrypt_type == ENCRYPT_TYPE_BLOWFISH && privacy.key)
	  ){
		GtkWidget *dialog;
		GtkResponseType result;
		gchar *msg;

		if(use_encrypt)
			msg = _("Configuration of encryption set active.\n"
				"Save ID information using the new settings now. OK?");
		else
			msg = _("Configuration of encryption set inactive.\n"
				"Save ID information using plain text now. OK?");

		dialog = gtk_message_dialog_new(NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_YES_NO,
						msg);
		result = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		if(result == GTK_RESPONSE_NO)
			return FALSE;

		write_idinfo = TRUE;
	}

	/* blowfish */
	if(use_encrypt && encrypt_type == ENCRYPT_TYPE_BLOWFISH){
		if(privacy.key){
			blowfish_key_free();
			blowfish_key_set(privacy.key);
			privacy.key = NULL;
			prefs_common.is_updated = TRUE;
		}
	}

	if(prefs_set_data_from_dialog(param))
		prefs_common.is_updated = TRUE;

	debug_print("prefs_common.is_updated: %d\n", prefs_common.is_updated);
	prefs_common_write_config();
	if(write_idinfo)
		main_window_save(prefs_dialog.parent, TRUE);

	prefs_dialog.is_finished = FALSE;

	return TRUE;
}

static void prefs_common_cancel(void)
{
	prefs_dialog.is_finished = TRUE;
}

static void prefs_common_privacy_create()
{
	GtkWidget *vbox;
	GtkWidget *vbox_spc;
	GtkWidget *hbox;
	GtkWidget *hbox_spc;
	GtkWidget *frame;
	GtkWidget *use_encrypt_chkbtn;
	GtkWidget *encrypt_type_combo;
#if USE_GPGME
	GtkWidget *gpg_keyid_btn;
	GtkWidget *keyid_entry;
#endif /* USE_GPGME */
	GtkWidget *use_encrypt_label;
	GtkWidget *bf_key_btn;

	vbox = gtk_vbox_new(FALSE, VSPACING);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(prefs_dialog.notebook), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), VBOX_BORDER);

	/* frame */
	frame = gtk_frame_new(_("Protection of Personal Infomation"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);
	gtk_frame_set_label_align(GTK_FRAME(frame), 0.0, 0.5);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), FRAME_VBOX_BORDER);

	hbox = gtk_hbox_new(FALSE, 8);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	/* use encrypt */
	use_encrypt_chkbtn = gtk_check_button_new_with_label
		(_("Encrypt when saving personal infomation"));
	gtk_widget_show(use_encrypt_chkbtn);
	gtk_box_pack_start(GTK_BOX(hbox), use_encrypt_chkbtn, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT(use_encrypt_chkbtn), "toggled",
			 G_CALLBACK(use_encrypt_toggled_cb), NULL);
	
	/* box space */
	hbox = gtk_hbox_new(FALSE, 8);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	hbox_spc = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox_spc);
	gtk_box_pack_start(GTK_BOX(hbox), hbox_spc, FALSE, FALSE, 0);
	gtk_widget_set_size_request(hbox_spc, 12, -1);

	/* vbox space */
	vbox_spc = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox_spc);
	gtk_box_pack_start(GTK_BOX(hbox), vbox_spc, FALSE, FALSE, 0);

	/* encrypt type dropdown */
	hbox = gtk_hbox_new(FALSE, 8);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox_spc), hbox, FALSE, FALSE, 0);

	encrypt_type_combo = prefs_common_encrypt_type_create();
	gtk_widget_show(encrypt_type_combo);
	gtk_box_pack_start(GTK_BOX(hbox), encrypt_type_combo, FALSE, FALSE, 0);
	SET_TOGGLE_SENSITIVE(use_encrypt_chkbtn, encrypt_type_combo);
	g_signal_connect(G_OBJECT(encrypt_type_combo), "changed",
			 G_CALLBACK(encrypt_type_changed_cb),
			 NULL);

#if USE_GPGME
	/* for GnuPG */
	gpg_keyid_btn = gtk_button_new_with_label(_("Select secret key..."));
	gtk_box_pack_start(GTK_BOX(hbox), gpg_keyid_btn, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(gpg_keyid_btn), "clicked",
			 G_CALLBACK(gpg_keyid_clicked_cb), NULL);
	SET_TOGGLE_SENSITIVE(use_encrypt_chkbtn, gpg_keyid_btn);
#endif /* USE_GPGME */

	/* for Blowfish */
	bf_key_btn = gtk_button_new_with_label(_("Setting Key..."));
	gtk_box_pack_start(GTK_BOX(hbox), bf_key_btn, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(bf_key_btn), "clicked",
			 G_CALLBACK(blowfish_key_clicked_cb), NULL);
	SET_TOGGLE_SENSITIVE(use_encrypt_chkbtn, bf_key_btn);

	/* attention desc. */
	hbox = gtk_hbox_new(FALSE, 8);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox_spc), hbox, FALSE, FALSE, 0);

	use_encrypt_label = gtk_label_new("");
		(_("You need the passphrase of this secret key on next startup."));
	gtk_widget_show(use_encrypt_label);
	gtk_box_pack_start(GTK_BOX(hbox), use_encrypt_label, FALSE, FALSE, 0);
	gtk_label_set_justify(GTK_LABEL(use_encrypt_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap(GTK_LABEL(use_encrypt_label), TRUE);
	gtkutils_widget_set_font(use_encrypt_label, -1, -1, PANGO_SCALE_SMALL);
	SET_TOGGLE_SENSITIVE(use_encrypt_chkbtn, use_encrypt_label);

#if USE_GPGME
	/* keyid entry */
	keyid_entry = gtk_entry_new();
#endif /* USE_GPGME */

	privacy.use_encrypt_chkbtn	= use_encrypt_chkbtn;
	privacy.encrypt_type_combo	= encrypt_type_combo;
#if USE_GPGME
	privacy.gpg_keyid_btn		= gpg_keyid_btn;
	privacy.keyid_entry		= keyid_entry;
#endif /* USE_GPGME */
	privacy.bf_key_btn		= bf_key_btn;
	privacy.use_encrypt_label	= use_encrypt_label;
	privacy.key			= NULL;
}

static GtkWidget *prefs_common_encrypt_type_create(void)
{
	GtkWidget *dropdown;
	GtkListStore *model;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;

	model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	dropdown = gtk_combo_box_new_with_model(GTK_TREE_MODEL(model));

	gtk_list_store_append(model, &iter);
	gtk_list_store_set(model, &iter,
			   0, _("Use Blowfish"),
			   1, ENCRYPT_TYPE_BLOWFISH,
			   -1);
#if USE_GPGME
	gtk_list_store_append(model, &iter);
	gtk_list_store_set(model, &iter,
			   0, _("Use GnuPG"),
			   1, ENCRYPT_TYPE_GNUPG,
			   -1);
#endif /* USE_GPGME */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(dropdown), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(dropdown), renderer,
				       "text", 0, NULL);

	return dropdown;
}

static void encrypt_type_changed_cb(GtkComboBox *combo_box, gpointer data)
{
	prefs_common_encrypt_toggled();
}

static void use_encrypt_toggled_cb(GtkWidget *widget, gpointer data)
{
	prefs_common_encrypt_toggled();
}

static void prefs_common_encrypt_toggled(void)
{
	GtkComboBox *combo_box = GTK_COMBO_BOX(privacy.encrypt_type_combo);
	GtkToggleButton *toggle = GTK_TOGGLE_BUTTON(privacy.use_encrypt_chkbtn);
	GtkTreeIter iter;
	GtkTreeModel *model;
	EncryptType encrypt_type;

	if(!gtk_combo_box_get_active_iter(combo_box, &iter)){
#if USE_GPGME
		gtk_widget_hide(privacy.gpg_keyid_btn);
#endif /* USE_GPGME */
		gtk_widget_hide(privacy.bf_key_btn);
		return;
	}

	model = gtk_combo_box_get_model(combo_box);
	gtk_tree_model_get(model, &iter,
			   1, &encrypt_type,
			   -1);

	switch(encrypt_type){
	case ENCRYPT_TYPE_BLOWFISH:
#if USE_GPGME
		gtk_widget_hide(privacy.gpg_keyid_btn);
#endif /* USE_GPGME */

		gtk_label_set_label
			(GTK_LABEL(privacy.use_encrypt_label),
			 _("You need a key on next startup."));
		gtk_widget_set_sensitive
			(privacy.bf_key_btn,
			 gtk_toggle_button_get_active(toggle) ? TRUE
							      : FALSE);
		gtk_widget_show(privacy.bf_key_btn);

		break;
#if USE_GPGME
	case ENCRYPT_TYPE_GNUPG:
		gtk_widget_hide(privacy.bf_key_btn);

		gtk_label_set_label
			(GTK_LABEL(privacy.use_encrypt_label),
			_("You need the passphrase of this secret key on next startup."));
		gtk_widget_set_sensitive
			(privacy.gpg_keyid_btn,
			 gtk_toggle_button_get_active(toggle) ? TRUE
							      : FALSE);
		gtk_widget_show(privacy.gpg_keyid_btn);

		break;
#endif /* USE_GPGME */
	default:
#if USE_GPGME
		gtk_widget_hide(privacy.gpg_keyid_btn);
#endif /* USE_GPGME */
		gtk_widget_hide(privacy.bf_key_btn);
		
		break;
	}
}

#if USE_GPGME
static void gpg_keyid_clicked_cb(void)
{
	const gchar *cur_keyid;
	const gchar *new_keyid;

	cur_keyid = gtk_entry_get_text(GTK_ENTRY(privacy.keyid_entry));
	new_keyid = gnupg_select_key_open(cur_keyid);
	if(new_keyid){
		gtk_entry_set_text(GTK_ENTRY(privacy.keyid_entry), new_keyid);
	}
}
#endif /* USE_GPGME */

static void blowfish_key_clicked_cb(void)
{
	gchar *key = NULL;

	if(blowfish_input_key_open(&key)){
		if(privacy.key){
			memset(privacy.key, 0, strlen(privacy.key));
			g_free(privacy.key);
		}
		privacy.key = key;
	}
}

static void prefs_common_other_create()
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *uri_entry;

	vbox = gtk_vbox_new(FALSE, VSPACING);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(prefs_dialog.notebook), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), VBOX_BORDER);

	frame = gtk_frame_new(_("External command (%s will be replaced with file name / URI)"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);
	gtk_frame_set_label_align(GTK_FRAME(frame), 0.0, 0.5);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), FRAME_VBOX_BORDER);

	hbox = gtk_hbox_new(FALSE, 32);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new(_("Web browser"));
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	uri_entry = gtk_entry_new();
	gtk_widget_show(uri_entry);
	gtk_box_pack_start(GTK_BOX(hbox), uri_entry, TRUE, TRUE, 0);

	other.uri_entry		= uri_entry;
}
