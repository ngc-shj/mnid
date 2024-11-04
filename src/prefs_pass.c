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
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef USE_CRACK
#  include <packer.h>
#endif /* USE_CRACK */

#include "prefs_common.h"
#include "prefs_pass.h"
#include "pass.h"
#include "utils.h"

static struct PrefsPassWindow {
	GtkWidget *dialog;

	GtkWidget *pass_entry;

	GtkWidget *use_char_toggle[N_PASS_USING_CHAR];
	/*
	GtkWidget *upper_toggle;
	GtkWidget *lower_toggle;
	GtkWidget *numeric_toggle;
	GtkWidget *symbol_toggle;
	*/
	GtkWidget *avoid_ambiguous_toggle;

	GtkWidget *digit_spin;
	GtkObject *digit_adj;

	GtkWidget *generate_btn;

	GtkWidget *progress_bar;

	PassInfo *cur_passinfo;
	PassInfo *new_passinfo;

	gboolean edit_finished;
} self;

static void prefs_pass_create		(void);
static void prefs_pass_ok		(void);
static void prefs_pass_cancel		(void);

static void prefs_pass_passinfo_to_dialog	(PassInfo	*passinfo);
static gboolean prefs_pass_dialog_to_passinfo	(PassInfo      **passinfo);

/* callback functions */
static gint dialog_key_press_event_cb	(GtkWidget	*widget,
					 GdkEventKey	*event,
					 gpointer	 data);
static void dialog_response_cb		(GtkDialog	*dialog,
					 gint		 response_id,
					 gpointer	 data);
static void digit_spin_value_changed_cb	(GtkSpinButton	*spin_button,
					 gpointer	 data);
static void generate_button_clicked_cb	(void);
static gint password_key_press_event_cb	(GtkWidget	*widget,
					 GdkEventKey	*event,
					 gpointer	 data);
static void password_quality_meter_cb	(GtkEditable	*editable,
					 gpointer	 data);

PassInfo *prefs_pass_open(PassInfo *passinfo)
{
	PassInfo *new_passinfo;

	if(!self.dialog)
		prefs_pass_create();

	prefs_pass_passinfo_to_dialog(passinfo);

	self.cur_passinfo = passinfo;
	self.new_passinfo = NULL;
	self.edit_finished = FALSE;

	while(self.edit_finished == FALSE)
		gtk_dialog_run(GTK_DIALOG(self.dialog));

	gtk_widget_hide(self.dialog);

	new_passinfo = self.new_passinfo;
	self.new_passinfo = NULL;
	self.cur_passinfo = NULL;

	if(new_passinfo)
		debug_print("new passinfo created.\n");

	return new_passinfo;
}

static void prefs_pass_create(void)
{
	GtkWidget *dialog;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *pass_entry;
	GtkWidget *frame;
	GtkWidget *frame_vbox;
	GtkWidget *btn_hbox;
	GtkWidget *use_char_toggle[N_PASS_USING_CHAR];
	/*
	GtkWidget *upper_toggle;
	GtkWidget *lower_toggle;
	GtkWidget *numeric_toggle;
	GtkWidget *symbol_toggle;
	*/
	GtkWidget *avoid_ambiguous_toggle;
	GtkWidget *digit_spin;
	GtkObject *digit_adj;
	GtkWidget *generate_btn;
	GtkWidget *progress_bar;

	gchar *buf;
	gint i;

        /* dialog */
	buf = g_strdup_printf("%s - %s", _("Password Generation"), FORMAL_NAME);
	dialog = gtk_dialog_new_with_buttons
			(buf,
			 NULL,
			 GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
			 GTK_STOCK_OK, GTK_RESPONSE_OK,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 NULL);
	g_free(buf);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	gtk_container_set_border_width(GTK_CONTAINER(dialog), WIN_BORDER);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_widget_realize(dialog);

	vbox = GTK_DIALOG(dialog)->vbox;
	gtk_box_set_spacing(GTK_BOX(vbox), VSPACING);
	
	/* pass */
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic(_("_Password"));
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	pass_entry = gtk_entry_new();
	gtk_widget_show(pass_entry);
	gtk_box_pack_start(GTK_BOX(hbox), pass_entry, TRUE, TRUE, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), pass_entry);

	/* generate */
	generate_btn = gtk_button_new_with_mnemonic(_("_Generate"));
	gtk_widget_show(generate_btn);
	gtk_box_pack_start(GTK_BOX(hbox), generate_btn, FALSE, FALSE, 0);
	
	/* frame */
	frame = gtk_frame_new(_("Auto Generation"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);
	gtk_frame_set_label_align(GTK_FRAME(frame), 0.0, 0.5);

	frame_vbox = gtk_vbox_new(FALSE, VSPACING_NARROW);
	gtk_widget_show(frame_vbox);
	gtk_container_add(GTK_CONTAINER(frame), frame_vbox);
	gtk_container_set_border_width(GTK_CONTAINER(frame_vbox), FRAME_VBOX_BORDER);

	/* upper / lower / numeric / symbol */
	table = gtk_table_new(3, 2, FALSE);
	gtk_widget_show(table);
	gtk_box_pack_start(GTK_BOX(frame_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 8);

	/* use chars */
	label = gtk_label_new(_("Using Characters"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label,
			 0, 1, 0, 1,
			 GTK_FILL, 0, 0, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	btn_hbox = gtk_hbox_new(TRUE, 4);
	gtk_widget_show(btn_hbox);
	gtk_table_attach(GTK_TABLE(table), btn_hbox,
			 1, 2, 0, 1,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);

	for(i = 0; i < N_PASS_USING_CHAR; i++){
		const gchar *tmp = NULL;

		switch(i){
		case PASS_UPPER:
			tmp = _("_Upper Case");
			break;
		case PASS_LOWER:
			tmp = _("_Lower Case");
			break;
		case PASS_NUMERIC:
			tmp = _("_Numeric");
			break;
		case PASS_SYMBOL:
			tmp = _("Sy_mbol");
			break;
		}
		use_char_toggle[i] =
			gtk_toggle_button_new_with_mnemonic(tmp);
		gtk_widget_show(use_char_toggle[i]);
		gtk_box_pack_start(GTK_BOX(btn_hbox),
				   use_char_toggle[i], FALSE, TRUE, 0);
	}

	/* avoid ambiguous */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);

	avoid_ambiguous_toggle = gtk_check_button_new_with_mnemonic
					(_("_Avoid ambiguous"));
	gtk_widget_show(avoid_ambiguous_toggle);
	gtk_box_pack_start(GTK_BOX(hbox),
			   avoid_ambiguous_toggle, FALSE, TRUE, 0);
	gtk_table_attach(GTK_TABLE(table), hbox,
			 1, 2, 1, 2,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);

	/* digit number */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_table_attach(GTK_TABLE(table), hbox,
			 1, 2, 2, 3,
			 GTK_EXPAND | GTK_FILL, 0, 0, 0);

	digit_adj = gtk_adjustment_new((gdouble)MNID_PASS_DEFAULT,
				       (gdouble)MNID_PASS_DEFAULT,
				       (gdouble)MNID_PASS_MAX,
				       1.0, 1.0, 0.0);
	digit_spin = gtk_spin_button_new(GTK_ADJUSTMENT(digit_adj), 1, 0);
	gtk_widget_show(digit_spin);
	gtk_box_pack_start(GTK_BOX(hbox), digit_spin, FALSE, FALSE, 0);
	gtk_widget_set_size_request(digit_spin, 64, -1);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(digit_spin), TRUE);

	label = gtk_label_new_with_mnemonic(_("_Digit number"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label,
			 0, 1, 2, 3,
			 GTK_FILL, 0, 0, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), digit_spin);

	/* password quality */
	frame = gtk_frame_new(_("Password quality"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);

	hbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), FRAME_VBOX_BORDER);

	progress_bar = gtk_progress_bar_new();
	gtk_widget_show(progress_bar);
	gtk_box_pack_start(GTK_BOX(hbox), progress_bar, FALSE, TRUE, 0);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);

	/* signals */
	g_signal_connect(G_OBJECT(dialog), "key_press_event",
			 G_CALLBACK(dialog_key_press_event_cb), NULL);
	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(dialog_response_cb), NULL);

	g_signal_connect(G_OBJECT(generate_btn), "clicked",
			 G_CALLBACK(generate_button_clicked_cb), NULL);

        g_signal_connect(G_OBJECT(pass_entry), "key_press_event",
                         G_CALLBACK(password_key_press_event_cb), NULL);
        g_signal_connect(G_OBJECT(pass_entry), "changed",
                         G_CALLBACK(password_quality_meter_cb), NULL);

	g_signal_connect(G_OBJECT(digit_spin), "value-changed",
			 G_CALLBACK(digit_spin_value_changed_cb), NULL);
 
	/* pointer */
	self.dialog		= dialog;
	self.pass_entry		= pass_entry;
	self.generate_btn	= generate_btn;
	self.progress_bar	= progress_bar;
	for(i = 0; i < N_PASS_USING_CHAR; i++)
		self.use_char_toggle[i] = use_char_toggle[i];
	self.avoid_ambiguous_toggle = avoid_ambiguous_toggle;
	self.digit_spin		= digit_spin;
	self.digit_adj		= digit_adj;
}

static void prefs_pass_passinfo_to_dialog(PassInfo *passinfo)
{
	gint i;
	gint val;

	for(i = 0; i < N_PASS_USING_CHAR; i++){
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(self.use_char_toggle[i]),
		 			   passinfo ? passinfo->use_char[i]
						    : FALSE);
	}

	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON(self.avoid_ambiguous_toggle),
		 passinfo ? passinfo->avoid_ambiguous
		 	  : FALSE);

	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON(self.digit_spin),
		 passinfo ? passinfo->digit
		 	  : MNID_PASS_DEFAULT);

	val = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(self.digit_spin));
	gtk_entry_set_max_length(GTK_ENTRY(self.pass_entry), val);
	gtk_entry_set_text(GTK_ENTRY(self.pass_entry),
			   passinfo ? passinfo->pass ? passinfo->pass
			   			     : ""
			   	    : "");
}

static gboolean prefs_pass_dialog_to_passinfo(PassInfo **passinfo)
{
	const gchar *pass;
	gboolean use_char[N_PASS_USING_CHAR];
	gboolean avoid_ambiguous;
	gint digit;
	gint i;
	gboolean is_updated;
	gchar *error_msg = NULL;

	*passinfo = NULL;

	pass = gtk_entry_get_text(GTK_ENTRY(self.pass_entry));
	if(!pass || *pass == '\0'){
		error_msg = _("Password is not specified.");
		goto error;
	}

	for(i = 0; i < N_PASS_USING_CHAR; i++)
		use_char[i] = gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON(self.use_char_toggle[i]));

	avoid_ambiguous = gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON(self.avoid_ambiguous_toggle));

	digit = gtk_spin_button_get_value_as_int
			(GTK_SPIN_BUTTON(self.digit_spin));


	if(self.cur_passinfo)
		*passinfo = pass_copy(self.cur_passinfo);
	else
		*passinfo = pass_new();

	is_updated = pass_update_value(*passinfo, pass, use_char,
				       avoid_ambiguous, digit);
	if(!is_updated){
		pass_free(*passinfo);
		*passinfo = NULL;
	}

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

		return FALSE;
	}

	return TRUE;
}

/* callback functions */

static gint dialog_key_press_event_cb(GtkWidget *widget, GdkEventKey *event,
				      gpointer data)
{
	if(event && event->keyval == GDK_Escape){
		gtk_dialog_response(GTK_DIALOG(self.dialog),
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
		prefs_pass_ok();
		break;
	case GTK_RESPONSE_CANCEL:
		prefs_pass_cancel();
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		gtk_dialog_response(GTK_DIALOG(self.dialog),
				    GTK_RESPONSE_CANCEL);
		break;
	case GTK_RESPONSE_NONE:
		break;
	default:
		break;
	}
}

static void prefs_pass_ok(void)
{
	PassInfo *passinfo;

	if(prefs_pass_dialog_to_passinfo(&passinfo)){
		self.new_passinfo = passinfo;
		self.edit_finished = TRUE;
	}
}

static void prefs_pass_cancel(void)
{
	self.new_passinfo = NULL;
	self.edit_finished = TRUE;
}

static void generate_button_clicked_cb(void)
{
	static gchar *tbl[] = {
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		"abcdefghijklmnopqrstuvwxyz",
		"0123456789",
		"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~", 
	};
	GRand *seed;
	gchar buf_pass[MNID_PASS_MAX + 1];
	gboolean use_char[N_PASS_USING_CHAR];
	gboolean avoid_ambiguous;
	gint digit;
	gint i;
	gboolean is_selected;
	gint type;
	gint pid;

	for(i = 0; i < N_PASS_USING_CHAR; i++)
		use_char[i] =
			gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON(self.use_char_toggle[i]));

	avoid_ambiguous = gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON(self.avoid_ambiguous_toggle));

	digit = gtk_spin_button_get_value_as_int
			  (GTK_SPIN_BUTTON(self.digit_spin));

	is_selected = FALSE;
	for(i = 0; i< N_PASS_USING_CHAR; i++)
		if(use_char[i])
			is_selected = TRUE;

	if(!is_selected){
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new(NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_OK,
						"%s",
						_("Select use characters."));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return;
	}

	pid = getpid();
	seed = g_rand_new();

#ifdef USE_CRACK
	{
	gchar *val;
	do{
#endif /* USE_CRACK */
	i = 0;
	while(i < digit){
		gchar pass_char;

		while(1){
			type = (g_rand_int(seed) ^ pid) % N_PASS_USING_CHAR;
			if(use_char[type])
				break;
		}
		pass_char = tbl[type][(g_rand_int(seed) ^ pid) %
					strlen(tbl[type])];

		if(avoid_ambiguous && strchr("0OlI1S5qg", pass_char))
			continue;

		buf_pass[i] = pass_char;
		i++;
		
	}
	buf_pass[i] = '\0';
#ifdef USE_CRACK
	val = FascistCheck(buf_pass, CRACKLIB_DICTPATH);
	debug_print("%s: %s\n", buf_pass, val);
	}while(val);
	}
#endif /* USE_CRACK */

	g_rand_free(seed);
	gtk_entry_set_text(GTK_ENTRY(self.pass_entry), buf_pass);
}

static void digit_spin_value_changed_cb(GtkSpinButton *spin_button,
					gpointer data)
{
	gint val;

	val = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(self.digit_spin));
	gtk_entry_set_max_length(GTK_ENTRY(self.pass_entry), val);
}

static gint password_key_press_event_cb(GtkWidget *widget, GdkEventKey *event,
					gpointer data)
{
	if(event){
		gboolean use_char[N_PASS_USING_CHAR];
		gint i;
		gunichar uc = gdk_keyval_to_unicode(event->keyval);

		if(event->state & GDK_CONTROL_MASK)
			return FALSE;

		for(i = 0; i < N_PASS_USING_CHAR; i++)
			use_char[i] = gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON(self.use_char_toggle[i]));

		if(g_unichar_isdigit(uc))
			return !use_char[PASS_NUMERIC];
		else if(g_unichar_isupper(uc))
			return !use_char[PASS_UPPER];
		else if(g_unichar_islower(uc))
			return !use_char[PASS_LOWER];
		else if(g_unichar_isgraph(uc))
			return !use_char[PASS_SYMBOL];
	}
	return FALSE;
}

static void password_quality_meter_cb(GtkEditable *editable, gpointer data)
{
	gchar *str = gtk_editable_get_chars(editable, 0, -1);
	guint strength;

	strength = password_quality_level(str);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(self.progress_bar),
					(strength / 100.0));

	g_free(str);
}

