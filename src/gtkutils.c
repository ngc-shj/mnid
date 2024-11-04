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

#include <glib.h>
#include <gtk/gtkwidget.h>

#include "gtkutils.h"
#include "utils.h"

void gtkutils_widget_set_font(GtkWidget *widget, PangoStyle style,
			      PangoWeight weight, double scale)
{
	static GtkWidget *window = NULL;
	PangoFontDescription *font_desc;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(widget->style != NULL);

	if(style < 0 && weight < 0 && scale < 0)
		return;

	if(!window)
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_widget_ensure_style(window);
	font_desc = pango_font_description_copy
					(window->style->font_desc);
	gtk_object_sink(GTK_OBJECT(window));

	/* style */
	if(style > -1)
		pango_font_description_set_style(font_desc, style);

	/* weight */
	if(weight > -1)
		pango_font_description_set_weight(font_desc, weight);

	/* scale */
	if(scale > -1){
		gint size;

		size = pango_font_description_get_size(font_desc);
		pango_font_description_set_size(font_desc, size * scale);
	}

	gtk_widget_modify_font(widget, font_desc);
	pango_font_description_free(font_desc);
}
