/*
 * $Id$
 *
 * Copyright (c) 2002-2003, Richard Eckart
 *
 *----------------------------------------------------------------------
 * This file is part of gtk-gnutella.
 *
 *  gtk-gnutella is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  gtk-gnutella is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gtk-gnutella; if not, write to the Free Software
 *  Foundation, Inc.:
 *      59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *----------------------------------------------------------------------
 */

#ifndef _gtk_gtk_missing_h_
#define _gtk_gtk_missing_h_

#include "gui.h"
#include <gtk/gtk.h>

#include "lib/prop.h"

/*
 * GtkWidget
 */
#ifdef USE_GTK1
#define gtk_widget_set_size_request(widget, width, height) \
	gtk_widget_set_usize((widget), (width), (height))
#endif

static inline void
widget_set_visible(GtkWidget *widget, gboolean visible)
{
	if (visible) {
		gtk_widget_show(widget);
	} else {
		gtk_widget_hide(widget);
	}
}

/*
 * GtkProgressBar
 *
 * Make Gtk1 and Gtk2 versions useable using the same interface.
 */
#ifdef USE_GTK1
#define gtk_progress_bar_set_fraction(pb, val) \
    gtk_progress_set_percentage(GTK_PROGRESS(pb), (val))
void gtk_progress_bar_set_text(GtkProgressBar *pb, const gchar *text);
gint gtk_paned_get_position(GtkPaned *paned);
#endif

/*
 * GtkSpinButton
 */
#ifdef USE_GTK1
#define gtk_spin_button_get_value(w) \
    _gtk_spin_button_get_value(w)
#endif
gdouble _gtk_spin_button_get_value(GtkSpinButton *);

/*
 * GtkCList
 */
#ifdef USE_GTK1
void gtk_clist_set_column_name(GtkCList * clist, gint col, gchar * t);
GSList *clist_collect_data(GtkCList *clist, gboolean allow_null,
    GCompareFunc cfn);
#endif /* USE_GTK1 */

/*
 * GtkCTree
 */
#ifdef USE_GTK1
void gtk_ctree_fast_move (GtkCTree *ctree, GtkCTreeNode *node,
	GtkCTreeNode *new_sibling);
gint gtk_ctree_count_node_children(GtkCTree *ctree, GtkCTreeNode *parent);

#define GTK_CTREE_NODE_HAS_CHILDREN(n) \
    ((n) ? (GTK_CTREE_ROW(n)->children) != NULL : FALSE)
#define GTK_CTREE_NODE_SIBLING(n) \
    ((n) ? (GTK_CTREE_ROW(n)->sibling) : NULL)
#define GTK_CTREE_NODE_PARENT(n) \
    ((n) ? (GTK_CTREE_ROW(n)->parent) : NULL)
#endif /* USE_GTK1 */
										
/**
 * GtkPaned
 */
#ifdef USE_GTK1 /* USE_GTK1 */
GtkWidget *gtk_paned_get_child1(GtkPaned *paned);
GtkWidget *gtk_paned_get_child2(GtkPaned *paned);
#endif /* USE_GTK1 */

/**
 * GtkLabel
 */
void gtk_label_printf(GtkLabel *, const gchar *fmt, ...) G_GNUC_PRINTF(2, 3);

/**
 * GtkEntry
 */
void gtk_entry_printf(GtkEntry *, const gchar *fmt, ...) G_GNUC_PRINTF(2, 3);

/**
 * GtkEditable
 */
guint32 gtk_editable_get_value_as_uint(GtkEditable *editable);

/**
 * GtkCombo
 */
void widget_init_choices(GtkWidget * widget, GtkSignalFunc func,
		prop_def_t *def, gpointer user_data);

/**
 * GtkOptionMenu
 */
void option_menu_select_item_by_data(GtkOptionMenu *option_menu,
	gconstpointer data);
gpointer option_menu_get_selected_data(GtkOptionMenu *option_menu);
GtkWidget *menu_new_item_with_data(GtkMenu *menu, const gchar *label_text,
	gpointer data);

/**
 * GtkWidget
 */
void gtk_mass_widget_set_sensitive(GtkWidget *tl,
	const gchar * const list[], guint n, gboolean b);

/*
 * GtkTreeView
 */
#ifdef USE_GTK2
typedef void (*tree_view_motion_callback)(GtkTreeView *, GtkTreePath *);
typedef struct tree_view_motion tree_view_motion_t;

typedef gpointer (*tree_selection_get_data_func)(GtkTreeModel *, GtkTreeIter *);

GtkTreeIter *w_tree_iter_new(void);
GtkTreeIter *w_tree_iter_copy(GtkTreeIter *);
void w_tree_iter_free(GtkTreeIter *);
void ht_w_tree_iter_free(gpointer);

GSList *tree_selection_collect_data(GtkTreeSelection *,
		tree_selection_get_data_func, GCompareFunc);
tree_view_motion_t *tree_view_motion_set_callback(GtkTreeView *,
	tree_view_motion_callback, guint interval);
void tree_view_motion_clear_callback(GtkTreeView *, tree_view_motion_t *);
void tree_view_set_fixed_height_mode(GtkTreeView *, gboolean fixed);
void tree_model_iter_changed(GtkTreeModel *, GtkTreeIter *);
#endif /* USE_GTK2 */

gint gtk_main_flush(void);
GtkWidget *radiobutton_get_active_in_group(GtkRadioButton *);

void gtk_widget_fix_width(GtkWidget *, GtkWidget *, guint chars, guint extra);

#ifdef USE_GTK1
#define gtk_get_current_event_time() (GDK_CURRENT_TIME)
#endif

#endif	/* _gtk_gtk_missing_h_ */

/* vi: set ts=4 sw=4 cindent: */
