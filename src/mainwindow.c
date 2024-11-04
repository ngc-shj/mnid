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
 *   o mainwindow.c
 *   o prefs_filter.c
 *
 */

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#include "main.h"
#include "mainwindow.h"
#include "prefs_common.h"
#include "id.h"
#include "prefs_id.h"
#include "utils.h"
#include "about.h"
#include "version.h"

static void main_window_set_widgets	(MainWindow	*mainwin);

/* callback functions */
static void main_window_new_clicked	(GtkWidget	*widget,
					 gpointer	 data);
static void main_window_edit_clicked	(GtkWidget	*widget,
					 gpointer	 data);
static void main_window_copy_clicked	(GtkWidget	*widget,
					 gpointer	 data);
static void main_window_delete_clicked	(GtkWidget	*widget,
					 gpointer	 data);
static void main_window_copy_uid_clicked(GtkWidget	*widget,
					 gpointer	 data);
static void main_window_copy_pwd_clicked(GtkWidget	*widget,
					 gpointer	 data);
static void main_window_jump_uri_clicked(GtkWidget	*widget,
					 gpointer	 data);
static void main_window_toolbar_toggle	(GtkRadioAction	*action,
					 GtkRadioAction	*current,
			   		 gpointer	 data);
static void main_window_statusbar_toggle(GtkToggleAction
							*action,
			   		 gpointer	 data);
static void main_window_top		(GtkWidget	*widget,
					 gpointer	 data);
static void main_window_up		(GtkWidget	*widget,
					 gpointer	 data);
static void main_window_down		(GtkWidget	*widget,
					 gpointer	 data);
static void main_window_bottom		(GtkWidget	*widget,
					 gpointer	 data);
static gboolean main_window_button_press_event
					(GtkWidget	*widget,
					 GdkEventButton	*event,
					 gpointer	 data);
/*
static gboolean main_window_select_cursor_row
					(GtkTreeView	*treeview,
					 gboolean	 arg1,
					 gpointer	 data);
*/
static gboolean main_window_row_activated
					(GtkTreeView	*treeview,
					 GtkTreePath	*path,
					 GtkTreeViewColumn
					 		*column,
					 gpointer	 data);
static gint main_window_key_pressed	(GtkWidget	*widget,
					 GdkEventKey	*event,
					 gpointer	 data);

static void main_window_set_list	(MainWindow	*mainwin);
static void main_window_set_list_row	(MainWindow	*mainwin,
					 GtkTreeIter	*iter,
					 IDinfo		*idinfo,
					 gboolean	 move_view);
static const IDinfo *main_window_get_cursor_idinfo
					(MainWindow	*mainwin);
static void main_window_up_row		(MainWindow	*mainwin,
					 gboolean	 move_top);
static void main_window_down_row	(MainWindow	*mainwin,
					 gboolean	 move_bottom);
/*
static void main_window_save		(MainWindow	*mainwin);

*/
static gint main_window_close_cb	(GtkWidget	*widget,
					 GdkEventAny	*event,
					 gpointer	 data);

/* menubar - activate */
static void prefs_common_open_cb	(GtkWidget	*widget,
					 gpointer	 data);
static void app_exit_cb			(GtkWidget	*widget,
					 gpointer	 data);
static void app_save_cb			(GtkWidget	*widget,
					 gpointer	 data);

/* menu */
static GtkActionEntry mainwin_entries[] =
{
	{"FileMenu", NULL, N_("_File")},
	{"Save", GTK_STOCK_SAVE, N_("_Save"),
		"<control>S", N_("Save ID information"), G_CALLBACK(app_save_cb)},
	{"Exit", GTK_STOCK_QUIT, N_("E_xit"),
		"<control>Q", NULL, G_CALLBACK(app_exit_cb)},

	{"IDMenu", NULL, N_("_ID Info")},
	{"New", GTK_STOCK_NEW, N_("_New"),
		"<control>N", N_("New ID information"), G_CALLBACK(main_window_new_clicked)},
	{"Edit", GTK_STOCK_PROPERTIES, N_("_Edit"),
		"<control>E", N_("Edit ID information"), G_CALLBACK(main_window_edit_clicked)},
	{"Copy", GTK_STOCK_COPY, N_("_Copy"),
		"<control>C", N_("Copy ID information"), G_CALLBACK(main_window_copy_clicked)},
	{"Delete", GTK_STOCK_DELETE, N_("_Delete"),
		"<control>D", N_("Delete ID information"), G_CALLBACK(main_window_delete_clicked)},
	{"CopyUID", NULL, N_("Copy _User ID"),
		"<shift><control>U", N_("Copy User ID"), G_CALLBACK(main_window_copy_uid_clicked)},
	{"CopyPwd", NULL, N_("Copy _Password"),
		"<shift><control>P", N_("Copy Password"), G_CALLBACK(main_window_copy_pwd_clicked)},
	{"JumpToURI", NULL, N_("_Jump To URI"),
		"<shift><control>J", N_("Jump To URI"), G_CALLBACK(main_window_jump_uri_clicked)},


	{"View", NULL, N_("_View")},
	{"Toolbar", NULL, N_("Toolbar")},
	{"Statusbar", NULL, N_("Statusbar")},

	{"ConfigurationMenu", NULL, N_("Confi_guration")},
	{"Common preferences", GTK_STOCK_PREFERENCES,
		N_("_Common preferences..."),
		NULL, NULL, G_CALLBACK(prefs_common_open_cb)},

	{"HelpMenu", NULL, N_("_Help")},
	{"About", NULL, N_("_About"),
		NULL, NULL, G_CALLBACK(about_show)},
};

static GtkRadioActionEntry toolbar_entries[] =
{
	{"Toolbar Icons", NULL, N_("_Icons"),
		NULL, NULL, GTK_TOOLBAR_ICONS},
	{"Toolbar Icons and Text", NULL, N_("Icons _and Text"),
		NULL, NULL, GTK_TOOLBAR_BOTH},
	{"Toolbar Text", NULL, N_("_Text"),
		NULL, NULL, GTK_TOOLBAR_TEXT},
	{"Toolbar None", NULL, N_("_None"),
		NULL, NULL, -1},
};

static GtkToggleActionEntry statusbar_entries[] =
{
	{"Statusbar", NULL, N_("Statusbar"),
		NULL, NULL, G_CALLBACK(main_window_statusbar_toggle), TRUE},
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='Save'/>"
"      <menuitem action='Exit'/>"
"    </menu>"
"    <menu action='IDMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Edit'/>"
"      <menuitem action='Copy'/>"
"      <menuitem action='Delete'/>"
"      <separator/>"
"      <menuitem action='CopyUID'/>"
"      <menuitem action='CopyPwd'/>"
"      <menuitem action='JumpToURI'/>"
"    </menu>"
"    <menu action='View'>"
"      <menu action='Toolbar'>"
"        <menuitem action='Toolbar Icons'/>"
"        <menuitem action='Toolbar Icons and Text'/>"
"        <menuitem action='Toolbar Text'/>"
"        <menuitem action='Toolbar None'/>"
"      </menu>"
"      <menuitem action='Statusbar'/>"
"    </menu>"
"    <menu action='ConfigurationMenu'>"
"      <menuitem action='Common preferences'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"  <toolbar name='Toolbar'>"
"    <toolitem action='Save'/>"
"    <separator/>"
"    <toolitem action='New'/>"
"    <toolitem action='Edit'/>"
"    <toolitem action='Copy'/>"
"    <toolitem action='Delete'/>"
/*
"    <separator/>"
"    <toolitem action='CopyUID'/>"
"    <toolitem action='CopyPwd'/>"
*/
"  </toolbar>"
"  <popup action='IDPopup'>"
"    <menuitem action='Edit'/>"
"    <menuitem action='Copy'/>"
"    <menuitem action='Delete'/>"
"    <separator/>"
"    <menuitem action='CopyUID'/>"
"    <menuitem action='CopyPwd'/>"
"    <menuitem action='JumpToURI'/>"
"  </popup>"
"</ui>";

static GtkUIManager *ui_manager_create		(MainWindow	*mainwin,
						 GtkWidget	*window);
static gchar *menu_translate_func		(const gchar	*path,
						 gpointer	 func_data);

MainWindow *main_window_create(void)
{
	MainWindow *mainwin;
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *vbox_body;
	GtkUIManager *ui_manager;
	GtkAction *action;
	GtkWidget *menubar;
	GtkWidget *toolbar;
	GtkWidget *statusbar;
	GtkWidget *id_popup;

	GtkWidget *hbox;
	GtkWidget *scrolledwin;
	GtkWidget *treeview;

	GtkWidget *btn_vbox;
	GtkWidget *spc_vbox;
	GtkWidget *top_btn;
	GtkWidget *up_btn;
	GtkWidget *down_btn;
	GtkWidget *bottom_btn;

	GtkWidget *btn_hbox;
	GtkWidget *new_btn;
	GtkWidget *edit_btn;
	GtkWidget *copy_btn;
	GtkWidget *del_btn;

	GtkTreeViewColumn *column;
	GtkTreeSelection *select;
	GtkCellRenderer *renderer;
	GtkListStore *model;

	debug_print("Creating main window...\n");
	mainwin = g_new0(MainWindow, 1);

	/* main window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), FORMAL_NAME" "VERSION);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_wmclass(GTK_WINDOW(window), "main_window", PACKAGE);
	gtk_widget_set_size_request(window, 180 * 3, 180 * 2);
	gtk_widget_realize(window);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* menu bar */
	ui_manager = ui_manager_create(mainwin, window);
	menubar = gtk_ui_manager_get_widget(ui_manager, "/MainMenu");
	gtk_widget_show(menubar);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, TRUE, 0);

	/* tool bar */
	toolbar = gtk_ui_manager_get_widget(ui_manager, "/Toolbar");
	gtk_widget_show(toolbar);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);

	/* popup menu */
	id_popup = gtk_ui_manager_get_widget(ui_manager, "/IDPopup");
	gtk_widget_hide(id_popup);

	/* status bar */
	statusbar = gtk_statusbar_new();
	gtk_widget_show(statusbar);
	gtk_box_pack_end(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);
	
	/* vbox that contains body */
	vbox_body = gtk_vbox_new(FALSE, VSPACING_NARROW);
	gtk_widget_show(vbox_body);
	gtk_container_set_border_width(GTK_CONTAINER(vbox_body), FRAME_VBOX_BORDER);
	gtk_box_pack_start(GTK_BOX(vbox), vbox_body, TRUE, TRUE, 0);

	/* signals */
	g_signal_connect(G_OBJECT(window), "delete_event",
			 G_CALLBACK(main_window_close_cb), mainwin);
	g_signal_connect(G_OBJECT(window), "key_press_event",
			 G_CALLBACK(main_window_key_pressed), mainwin);

	/* password list */
	hbox = gtk_hbox_new(FALSE, 8);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox_body), hbox, TRUE ,TRUE, 0);

	scrolledwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrolledwin);
	gtk_widget_set_size_request(scrolledwin, -1, 150);
	gtk_box_pack_start(GTK_BOX(hbox), scrolledwin, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
					    GTK_SHADOW_IN);

	/* Create the list model. */
	model = gtk_list_store_new(M_COLS,
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_POINTER);

	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_widget_show(treeview);
	gtk_container_add(GTK_CONTAINER(scrolledwin), treeview);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_selection_set_mode
			(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)),
			 GTK_SELECTION_BROWSE);

#define TREE_VIEW_APPEND_TEXT_COLUMN(__title, __pos, __width) \
{ \
	renderer = gtk_cell_renderer_text_new(); \
	column = gtk_tree_view_column_new_with_attributes(__title, \
							  renderer, "text", \
							  __pos, NULL); \
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column); \
	gtk_tree_view_column_set_sizing(column, \
					GTK_TREE_VIEW_COLUMN_FIXED); \
	gtk_tree_view_column_set_fixed_width(column, __width); \
	gtk_tree_view_column_set_resizable(column, TRUE); \
}

	TREE_VIEW_APPEND_TEXT_COLUMN(_("Name"), M_COL_NAME, 320);
	TREE_VIEW_APPEND_TEXT_COLUMN(_("User ID"), M_COL_UID, 80);

#undef TREE_VIEW_APPEND_TEXT_COLUMN

	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	gtk_tree_selection_set_mode(select, GTK_SELECTION_BROWSE);
	g_signal_connect(G_OBJECT(treeview), "button_press_event",
			 G_CALLBACK(main_window_button_press_event), mainwin);
	/*
	g_signal_connect(G_OBJECT(treeview), "select-cursor-row",
			 G_CALLBACK(main_window_select_cursor_row), mainwin);
	*/
	g_signal_connect(G_OBJECT(treeview), "row-activated",
			 G_CALLBACK(main_window_row_activated), mainwin);

	/* Up / Down */

	btn_vbox = gtk_vbox_new(FALSE, VSPACING);
	gtk_widget_show(btn_vbox);
	gtk_box_pack_start(GTK_BOX(hbox), btn_vbox, FALSE, FALSE, 0);

	/* Top button */
	top_btn = gtk_button_new_from_stock(GTK_STOCK_GOTO_TOP);
	gtk_widget_show(top_btn);
	gtk_box_pack_start(GTK_BOX(btn_vbox), top_btn, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(top_btn), "clicked",
			 G_CALLBACK(main_window_top), mainwin);
	
	/* space */
	spc_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(spc_vbox);
	gtk_box_pack_start(GTK_BOX(btn_vbox), spc_vbox, FALSE, FALSE, 2);

	/* Up button */
	up_btn = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
	gtk_widget_show(up_btn);
	gtk_box_pack_start(GTK_BOX(btn_vbox), up_btn, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(up_btn), "clicked",
			 G_CALLBACK(main_window_up), mainwin);
	
	/* Down button */
	down_btn = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
	gtk_widget_show(down_btn);
	gtk_box_pack_start(GTK_BOX(btn_vbox), down_btn, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(down_btn), "clicked",
			 G_CALLBACK(main_window_down), mainwin);
	
	/* space */
	spc_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(spc_vbox);
	gtk_box_pack_start(GTK_BOX(btn_vbox), spc_vbox, FALSE, FALSE, 2);

	/* Bottom button */
	bottom_btn = gtk_button_new_from_stock(GTK_STOCK_GOTO_BOTTOM);
	gtk_widget_show(bottom_btn);
	gtk_box_pack_start(GTK_BOX(btn_vbox), bottom_btn, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(bottom_btn), "clicked",
			 G_CALLBACK(main_window_bottom), mainwin);

	/* add / edit / copy / delete */

	hbox = gtk_hbox_new(FALSE, 4);
	/* gtk_widget_show(hbox); */
	/* gtk_box_pack_start(GTK_BOX(vbox_body), hbox, FALSE, FALSE, 0); */

	btn_hbox = gtk_hbox_new(TRUE, 4);
	gtk_widget_show(btn_hbox);
	gtk_box_pack_start(GTK_BOX(hbox), btn_hbox, FALSE, FALSE, 0);

	/* New button */
	new_btn = gtk_button_new_from_stock(GTK_STOCK_NEW);
	gtk_widget_show(new_btn);
	gtk_box_pack_start(GTK_BOX(btn_hbox), new_btn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(new_btn), "clicked",
			 G_CALLBACK(main_window_new_clicked), mainwin);

	/* Edit button */
	edit_btn = gtk_button_new_with_mnemonic(_("_Edit"));
	gtk_widget_show(edit_btn);
	gtk_box_pack_start(GTK_BOX(btn_hbox), edit_btn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(edit_btn), "clicked",
			 G_CALLBACK(main_window_edit_clicked), mainwin);

	/* Copy button */
	copy_btn = gtk_button_new_from_stock(GTK_STOCK_COPY);
	gtk_widget_show(copy_btn);
	gtk_box_pack_start(GTK_BOX(btn_hbox), copy_btn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(copy_btn), "clicked",
			 G_CALLBACK(main_window_copy_clicked), mainwin);

	/* Delete button */
	del_btn = gtk_button_new_from_stock(GTK_STOCK_DELETE);
	gtk_widget_show(del_btn);
	gtk_box_pack_start(GTK_BOX(btn_hbox), del_btn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(del_btn), "clicked",
			 G_CALLBACK(main_window_delete_clicked), mainwin);

	mainwin->window			= window;
	mainwin->menubar		= menubar;
	mainwin->toolbar		= toolbar;
	mainwin->id_popup		= id_popup;
	mainwin->statusbar		= statusbar;
	mainwin->treeview		= treeview;
	mainwin->model			= GTK_TREE_MODEL(model);
	mainwin->new_btn		= new_btn;
	mainwin->edit_btn		= edit_btn;
	mainwin->copy_btn		= copy_btn;
	mainwin->del_btn		= del_btn;
	mainwin->is_updated		= FALSE;

	/* set context IDs for status bar */
	mainwin->context_id = gtk_statusbar_get_context_id
				(GTK_STATUSBAR(statusbar), "Main Window");

	main_window_set_widgets(mainwin);

	/* show all */
	gtk_widget_show_all(window);

	/* statusbar */
	action = gtk_ui_manager_get_action(ui_manager,
					   "/MainMenu/View/Statusbar");
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),
				     prefs_common.show_statusbar);
	
	/* toolbar */
	switch(prefs_common.toolbar_style){
	case GTK_TOOLBAR_ICONS:
		action = gtk_ui_manager_get_action
				(ui_manager,
				 "/MainMenu/View/Toolbar/Toolbar Icons");
		break;
	case GTK_TOOLBAR_BOTH:
		action = gtk_ui_manager_get_action
				(ui_manager,
				 "/MainMenu/View/Toolbar/Toolbar Icons and Text");
		break;
	case GTK_TOOLBAR_TEXT:
		action = gtk_ui_manager_get_action
				(ui_manager,
				 "/MainMenu/View/Toolbar/Toolbar Text");
		break;
	case -1:
		action = gtk_ui_manager_get_action
				(ui_manager,
				 "/MainMenu/View/Toolbar/Toolbar None");
		break;
	}
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), TRUE);

	debug_print("done.\n");

	return mainwin;
}

void main_window_set_dialog(MainWindow *mainwin)
{
	GSList *cur;
	gint append_row = 0;

	gtk_tree_view_set_model(GTK_TREE_VIEW(mainwin->treeview), NULL);

	for(cur = prefs_common.idlist; cur; cur = cur->next){
		IDinfo *idinfo = (IDinfo *)cur->data;
		main_window_set_list_row(mainwin, NULL, idinfo, FALSE);
		append_row++;
	}

	/*
	if(!append_row)
		mainwin->is_updated = TRUE;
	*/

	gtk_tree_view_set_model(GTK_TREE_VIEW(mainwin->treeview),
				mainwin->model);
}

static void main_window_set_list_row(MainWindow *mainwin, GtkTreeIter *iter,
				     IDinfo *idinfo, gboolean move_view)
{
	GtkTreeModel *model = mainwin->model;
	GtkTreeIter new_iter;
	gchar *id_str[M_COLS] = {NULL, NULL};

	if(!idinfo){
		gtk_tree_model_get(model, iter,
				   M_COL_POINTER, &idinfo,
				   -1);
	}

	g_return_if_fail(idinfo != NULL);

	/* name */
	if(idinfo->name && *idinfo->name)
		id_str[M_COL_NAME] = g_strdup(idinfo->name);
	else
		id_str[M_COL_NAME] = "";

	/* uid */
	if(idinfo->uid && *idinfo->uid)
		id_str[M_COL_UID] = g_strdup(idinfo->uid);
	else
		id_str[M_COL_UID] = "";

	/* iter is null ---> new or copy */
	if(!iter){
		gtk_list_store_append(GTK_LIST_STORE(model), &new_iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &new_iter,
				   M_COL_NAME, id_str[M_COL_NAME],
				   M_COL_UID, id_str[M_COL_UID],
				   M_COL_POINTER, idinfo,
				   -1);
	}else{
		IDinfo *prev_idinfo;

		gtk_tree_model_get(model, iter,
				   M_COL_POINTER, &prev_idinfo,
				   -1);

		if(idinfo == prev_idinfo){
			gtk_list_store_set(GTK_LIST_STORE(model), iter,
				   	   M_COL_NAME, id_str[M_COL_NAME],
				   	   M_COL_UID, id_str[M_COL_UID],
					   M_COL_POINTER, idinfo,
					   -1);
		}
		else if(prev_idinfo){
			gtk_list_store_set(GTK_LIST_STORE(model), iter,
				   	   M_COL_NAME, id_str[M_COL_NAME],
				   	   M_COL_UID, id_str[M_COL_UID],
					   M_COL_POINTER, idinfo,
					   -1);
			id_free(prev_idinfo);
		}else{
			gtk_list_store_append(GTK_LIST_STORE(model), &new_iter);
			gtk_list_store_set(GTK_LIST_STORE(model), &new_iter,
					   M_COL_NAME, id_str[M_COL_NAME],
					   M_COL_UID, id_str[M_COL_UID],
					   M_COL_POINTER, idinfo,
					   -1);
		}
	}

	g_free(id_str[0]);
	g_free(id_str[1]);
}

static void main_window_set_list(MainWindow *mainwin)
{
	GtkTreeModel *model = mainwin->model;
	GtkTreeIter iter;
	gboolean valid;
	IDinfo *idinfo;

	g_slist_free(prefs_common.idlist);
	prefs_common.idlist = NULL;

	valid = gtk_tree_model_get_iter_first(model, &iter);
	while(valid){
		gtk_tree_model_get(model, &iter,
			  	   M_COL_POINTER, &idinfo, 
				   -1);
		prefs_common.idlist = g_slist_append(prefs_common.idlist,
						     idinfo);
		valid = gtk_tree_model_iter_next(model, &iter);
	}
}

static void main_window_new_clicked(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;
	IDinfo *idinfo;

	idinfo = prefs_id_open(NULL, PREFS_ID_MODE_NEW);

	if(idinfo){
		main_window_set_list_row(mainwin, NULL, idinfo, TRUE);
		main_window_set_list(mainwin);
		mainwin->is_updated = TRUE;
	}
}

static void main_window_edit_clicked(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;
	GtkTreeView *treeview = GTK_TREE_VIEW(mainwin->treeview);
	GtkTreePath *path = NULL;
	GtkTreeModel *model = mainwin->model;
	GtkTreeIter iter;
	IDinfo *idinfo, *new_idinfo;

	gtk_tree_view_get_cursor(treeview, &path, NULL);
	if(!path){
		debug_print("main_window_edit_clicked(): nothing selected\n");
		gtk_statusbar_push(GTK_STATUSBAR(mainwin->statusbar),
				   mainwin->context_id,
				   _("Select ID infomation."));
		return;
	}

	if(!gtk_tree_model_get_iter(model, &iter, path)){
		g_warning("gtk_tree_model_get_iter failed\n");
		gtk_tree_path_free(path);
		return;
	}
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter,
			   M_COL_POINTER, &idinfo, 
			   -1);

	g_return_if_fail(idinfo != NULL);

	new_idinfo = prefs_id_open(idinfo, PREFS_ID_MODE_EDIT);

	if(new_idinfo){
		main_window_set_list_row(mainwin, &iter, new_idinfo, TRUE);
		main_window_set_list(mainwin);
		mainwin->is_updated = TRUE;
	}
}

static void main_window_copy_clicked(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;
	GtkTreeView *treeview = GTK_TREE_VIEW(mainwin->treeview);
	GtkTreePath *path = NULL;
	GtkTreeModel *model = mainwin->model;
	GtkTreeIter iter;
	IDinfo *idinfo, *new_idinfo;

	gtk_tree_view_get_cursor(treeview, &path, NULL);
	if(!path){
		debug_print("main_window_copy_clicked(): nothing selected\n");
		gtk_statusbar_push(GTK_STATUSBAR(mainwin->statusbar),
				   mainwin->context_id,
				   _("Select ID infomation."));
		return;
	}

	if(!gtk_tree_model_get_iter(model, &iter, path)){
		g_warning("gtk_tree_model_get_iter failed\n");
		gtk_tree_path_free(path);
		return;
	}
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter,
			   M_COL_POINTER, &idinfo, 
			   -1);

	g_return_if_fail(idinfo != NULL);
	
	new_idinfo = prefs_id_open(idinfo, PREFS_ID_MODE_COPY);
	
	if(new_idinfo){
		main_window_set_list_row(mainwin, NULL, new_idinfo, TRUE);
		main_window_set_list(mainwin);
		mainwin->is_updated = TRUE;
	}
}

static void main_window_delete_clicked(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;
	GtkWidget *dialog;
	GtkResponseType result;
	GtkTreeView *treeview = GTK_TREE_VIEW(mainwin->treeview);
	GtkTreePath *path = NULL;
	GtkTreeModel *model = mainwin->model;
	GtkTreeIter iter;
	IDinfo *idinfo;

	gtk_tree_view_get_cursor(treeview, &path, NULL);
	if(!path){
		debug_print("main_window_delete_clicked(): nothing selected\n");
		gtk_statusbar_push(GTK_STATUSBAR(mainwin->statusbar),
				   mainwin->context_id,
				   _("Select ID infomation."));
		return;
	}

	dialog = gtk_message_dialog_new
			       (NULL,
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				_("Delete ID:\n"
				  "Do you really want to delete this ID?"));

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	if(result == GTK_RESPONSE_NO)
		return;

	if(!gtk_tree_model_get_iter(model, &iter, path)){
		g_warning("gtk_tree_model_get_iter failed\n");
		gtk_tree_path_free(path);
		return;
	}
	gtk_tree_model_get(model, &iter,
			   M_COL_POINTER, &idinfo, 
			   -1);
	id_free(idinfo);
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	prefs_common.idlist = g_slist_remove(prefs_common.idlist, idinfo);

	if(gtk_tree_path_prev(path))
		gtk_tree_view_set_cursor(treeview, path, NULL, FALSE);

	mainwin->is_updated = TRUE;
}

static void main_window_copy_uid_clicked(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;
	const IDinfo *idinfo;
	const gchar *uid;
	GtkClipboard *clipboard;

	if(!mainwin)
		return;

	idinfo = main_window_get_cursor_idinfo(mainwin);
	if(!idinfo)
		return;

	uid = idinfo->uid;
	if(!idinfo->uid || *idinfo->uid == '\0')
		return;

	clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
	gtk_clipboard_set_text(clipboard, uid, -1);
	clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(clipboard, uid, -1);
}

static void main_window_copy_pwd_clicked(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;
	const IDinfo *idinfo;
	const gchar *pass;
	GtkClipboard *clipboard;

	if(!mainwin)
		return;

	idinfo = main_window_get_cursor_idinfo(mainwin);
	if(!idinfo || !idinfo->passinfo)
		return;

	pass = idinfo->passinfo->pass;
	if(!pass || *pass == '\0')
		return;

	clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
	gtk_clipboard_set_text(clipboard, pass, -1);
	clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(clipboard, pass, -1);
}

static const IDinfo *main_window_get_cursor_idinfo(MainWindow *mainwin)
{
	GtkTreeView *treeview = GTK_TREE_VIEW(mainwin->treeview);
	GtkTreePath *path = NULL;
	GtkTreeModel *model = mainwin->model;
	GtkTreeIter iter;
	IDinfo *idinfo;

	gtk_tree_view_get_cursor(treeview, &path, NULL);
	if(!path){
		debug_print("%s(): nothing selected\n", __FUNCTION__);
		return NULL;
	}

	if(!gtk_tree_model_get_iter(model, &iter, path)){
		g_warning("gtk_tree_model_get_iter failed\n");
		gtk_tree_path_free(path);
		return NULL;
	}

	gtk_tree_model_get(model, &iter,
			   M_COL_POINTER, &idinfo,
			   -1);
	return idinfo;
}

static void main_window_jump_uri_clicked(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;
	const IDinfo *idinfo;
	const gchar *uri;

	if(!mainwin)
		return;

	idinfo = main_window_get_cursor_idinfo(mainwin);
	if(!idinfo)
		return;

	uri = idinfo->uri;
	if(!uri || *uri == '\0')
		return;

	open_uri(uri, prefs_common.uri_cmd);
}

static void main_window_toolbar_toggle(GtkRadioAction *action,
				       GtkRadioAction *current, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;
	gint value;

	value = gtk_radio_action_get_current_value(GTK_RADIO_ACTION(current));

	switch(value){
	case GTK_TOOLBAR_ICONS:
		gtk_toolbar_set_style(GTK_TOOLBAR(mainwin->toolbar),
				      GTK_TOOLBAR_ICONS);
		break;
	case GTK_TOOLBAR_TEXT:
		gtk_toolbar_set_style(GTK_TOOLBAR(mainwin->toolbar),
				      GTK_TOOLBAR_TEXT);
		break;
	case GTK_TOOLBAR_BOTH:
		gtk_toolbar_set_style(GTK_TOOLBAR(mainwin->toolbar),
				      GTK_TOOLBAR_BOTH);
		break;
	}

	if(value != -1){
		gtk_widget_show(mainwin->toolbar);
		gtk_widget_queue_resize(mainwin->toolbar);
	}else
		gtk_widget_hide(mainwin->toolbar);

	prefs_common.toolbar_style = value;
}

static void main_window_statusbar_toggle(GtkToggleAction *action, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	if(gtk_toggle_action_get_active(action)){
		gtk_widget_show(mainwin->statusbar);
		gtk_widget_queue_resize(mainwin->statusbar);
		prefs_common.show_statusbar = TRUE;
	}else{
		gtk_widget_hide(mainwin->statusbar);
		prefs_common.show_statusbar = FALSE;
	}
}

static void main_window_top(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	main_window_up_row(mainwin, TRUE);
}

static void main_window_up(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	main_window_up_row(mainwin, FALSE);
}

static void main_window_up_row(MainWindow *mainwin, gboolean move_top)
{
	GtkTreeView *treeview = GTK_TREE_VIEW(mainwin->treeview);
	GtkTreePath *path = NULL, *top_path = NULL;
	GtkTreeModel *model = mainwin->model;
	GtkTreeIter iter, top_iter, position;

	gtk_tree_view_get_cursor(treeview, &path, NULL);
	if(!path){
		debug_print("main_window_up(): nothing selected\n");
		return;
	}

	if(!gtk_tree_model_get_iter(model, &iter, path)){
		g_warning("gtk_tree_model_get_iter failed\n");
		gtk_tree_path_free(path);
		return;
	}

	if(!gtk_tree_model_get_iter_first(model, &top_iter)){
		debug_print("main_window_up().iter_first(): false\n");
		gtk_tree_path_free(path);
		return;
	}

	top_path = gtk_tree_model_get_path(model, &top_iter);
	if(!top_path){
		debug_print("main_window_up().get_path(top): false\n");
		gtk_tree_path_free(path);
		return;
	}

	if(gtk_tree_path_prev(path)){
		if(!move_top && gtk_tree_path_compare(top_path, path)){
			if(gtk_tree_model_get_iter(model, &position, path)){
				gtk_list_store_move_before
					(GTK_LIST_STORE(model), &iter,
					 &position);
				gtk_tree_view_set_cursor(treeview, path,
							 NULL, FALSE);
				main_window_set_list(mainwin);
				mainwin->is_updated = TRUE;
			}
		}else{
			gtk_list_store_move_after(GTK_LIST_STORE(model),
						  &iter, NULL);
			gtk_tree_view_set_cursor(treeview, top_path, NULL,
						 FALSE);
			main_window_set_list(mainwin);
			mainwin->is_updated = TRUE;
		}
	}

	gtk_tree_path_free(path);
	gtk_tree_path_free(top_path);
}

static void main_window_down(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	main_window_down_row(mainwin, FALSE);
}

static void main_window_bottom(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	main_window_down_row(mainwin, TRUE);
}

static void main_window_down_row(MainWindow *mainwin, gboolean move_bottom)
{
	GtkTreeView *treeview = GTK_TREE_VIEW(mainwin->treeview);
	GtkTreePath *path = NULL;
	GtkTreeModel *model = mainwin->model;
	GtkTreeIter iter, *position;

	gtk_tree_view_get_cursor(treeview, &path, NULL);
	if(!path){
		debug_print("main_window_down(): nothing selected\n");
		return;
	}

	if(!gtk_tree_model_get_iter(model, &iter, path)){
		g_warning("gtk_tree_model_get_iter failed\n");
		gtk_tree_path_free(path);
		return;
	}
	gtk_tree_path_free(path);

	position = gtk_tree_iter_copy(&iter);
	/* if there is next iter */
	if(gtk_tree_model_iter_next(model, position)){
		GtkTreePath *new_path = NULL;

		if(move_bottom){
			gtk_list_store_move_before(GTK_LIST_STORE(model), &iter,
						   NULL);

		}else
			gtk_list_store_move_after(GTK_LIST_STORE(model), &iter,
						  position);
		gtk_tree_view_get_cursor(treeview, &new_path, NULL);
		gtk_tree_view_set_cursor(treeview, new_path, NULL, FALSE);
		gtk_tree_path_free(new_path);
		
		main_window_set_list(mainwin);
		mainwin->is_updated = TRUE;
	}
	gtk_tree_iter_free(position);
}

static gboolean main_window_button_press_event(GtkWidget *widget,
					       GdkEventButton *event,
					       gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	if(!event)
		return FALSE;

	if(event->button == 3){
		gtk_menu_popup(GTK_MENU(mainwin->id_popup), NULL, NULL,
			       NULL, NULL, event->button, event->time);
	}
	return FALSE;
}

/*
static gboolean main_window_select_cursor_row(GtkTreeView *treeview,
					      gboolean arg1, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	debug_print("%% select-cursor-row! \n");
	main_window_edit_clicked(NULL, mainwin);
	return TRUE;
}
*/
static gboolean main_window_row_activated(GtkTreeView *treeview,
					  GtkTreePath *path,
					  GtkTreeViewColumn *column,
					  gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	main_window_edit_clicked(NULL, mainwin);
	return TRUE;
}

static gint main_window_key_pressed(GtkWidget *widget, GdkEventKey *event,
				    gpointer data)
{
	/*
	if(event && event->keyval == GDK_Escape){
		main_window_close_cb(widget, (GdkEventAny *)event, data);
		return FALSE;
	}
	*/
	return FALSE;
}

static gint main_window_close_cb(GtkWidget *widget, GdkEventAny *event,
				 gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	app_exit_cb(widget, mainwin);

	return TRUE;
}

void main_window_get_size(MainWindow *mainwin)
{
	GtkAllocation *allocation;

	allocation = &mainwin->window->allocation;
	
	if (allocation->width > 1 && allocation->height > 1) {
		prefs_common.mainwin_width	= allocation->width;
		prefs_common.mainwin_height	= allocation->height;
	}
}

void main_window_get_position(MainWindow *mainwin)
{
	gint x, y;
	gint sx, sy;

	sx = gdk_screen_width();
	sy = gdk_screen_height();

	gdk_window_get_root_origin(mainwin->window->window, &x, &y);

	/* for virtual desktop */
	x %= sx; if(x < 0) x = 0;
	y %= sy; if(y < 0) y = 0;

	prefs_common.mainwin_x = x;
	prefs_common.mainwin_y = y;
}

static void main_window_set_widgets(MainWindow *mainwin)
{
	gtk_window_move(GTK_WINDOW(mainwin->window),
			prefs_common.mainwin_x,
			prefs_common.mainwin_y);

	gtk_widget_queue_resize(mainwin->window);
}

static void prefs_common_open_cb(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	prefs_common_open(mainwin);
}

static void app_exit_cb(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	if(mainwin->is_updated){
		GtkWidget *dialog;
		GtkResponseType response_id;

		dialog = gtk_message_dialog_new
				(NULL,
				 GTK_DIALOG_MODAL,
				 GTK_MESSAGE_QUESTION,
				 GTK_BUTTONS_YES_NO,
				 _("ID infomation have been updated. "
				   "Save your change?"));
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

		response_id = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		switch(response_id){
		case GTK_RESPONSE_YES:
			main_window_save(mainwin, FALSE);
			break;
		case GTK_RESPONSE_NO:
			break;
		case GTK_RESPONSE_CANCEL:
			return;
			break;
		case GTK_RESPONSE_DELETE_EVENT:
			return;
			break;
		case GTK_RESPONSE_NONE:
			break;
		default:
			break;
		}
	}
	app_will_exit(NULL, mainwin);
}

static void app_save_cb(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;

	main_window_save(mainwin, FALSE);
}

void main_window_save(MainWindow *mainwin, gboolean is_force)
{

	if(mainwin->is_updated || is_force){
		prefs_id_write_config();
		gtk_statusbar_push(GTK_STATUSBAR(mainwin->statusbar),
				   mainwin->context_id,
				   _("Save ID information."));
		mainwin->is_updated = FALSE;
	}else{
		gtk_statusbar_push(GTK_STATUSBAR(mainwin->statusbar),
				   mainwin->context_id,
				   _("ID information have not been updated."));
	}
}

static GtkUIManager *ui_manager_create(MainWindow *mainwin, GtkWidget *window)
{
	GtkActionGroup *action_group;
	GtkUIManager *ui_manager;
	GtkAccelGroup *accel_group;
	GError *error;

	ui_manager = gtk_ui_manager_new();

	/* Menu Actions */
	action_group = gtk_action_group_new("MenuActions");
	gtk_action_group_set_translate_func(action_group, menu_translate_func,
					    NULL, NULL);
	gtk_action_group_add_actions(action_group, mainwin_entries,
				     G_N_ELEMENTS(mainwin_entries), mainwin);
	gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);

	/* Menu Toolbar Actions */
	action_group = gtk_action_group_new("ToolbarActions");
	gtk_action_group_set_translate_func(action_group, menu_translate_func,
					    NULL, NULL);
	gtk_action_group_add_radio_actions(action_group, toolbar_entries,
			G_N_ELEMENTS(toolbar_entries),
			GTK_TOOLBAR_BOTH,
			G_CALLBACK(main_window_toolbar_toggle),
			mainwin);
	gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);

	/* Menu Statusbar Actions */
	action_group = gtk_action_group_new("StatusbarActions");
	gtk_action_group_set_translate_func(action_group, menu_translate_func,
					    NULL, NULL);
	gtk_action_group_add_toggle_actions(action_group, statusbar_entries,
				     G_N_ELEMENTS(statusbar_entries),
				     mainwin);
	gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);

	accel_group = gtk_ui_manager_get_accel_group(ui_manager);
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	if(!gtk_ui_manager_add_ui_from_string(ui_manager, ui_description,
					      -1, &error)){
		g_message("building menus failed: %s", error->message);
		g_error_free(error);
		return NULL;
	}

	return ui_manager;
}

static gchar *menu_translate_func(const gchar *path, gpointer func_data)
{
	gchar *retval;

	retval = gettext(path);

	return retval;
}
