/*
 * $Id$
 *
 * Copyright (c) 2001-2003, Richard Eckart
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

#include "common.h"

RCSID("$Id$")

#include "gtk/gui.h"
#include "interface-glade.h"
#include "gtk/uploads.h"
#include "gtk/uploads_common.h"
#include "gtk/columns.h"
#include "gtk/notebooks.h"
#include "gtk/settings.h"

#include "if/bridge/ui2c.h"

#include "lib/host_addr.h"
#include "lib/glib-missing.h"
#include "lib/iso3166.h"
#include "lib/tm.h"
#include "lib/utf8.h"
#include "lib/walloc.h"
#include "lib/override.h"	/* Must be the last header included */

#define UPDATE_MIN	60		/**< Update screen every minute at least */

static gboolean uploads_remove_lock = FALSE;
static guint uploads_rows_done = 0;

static gint find_row(gnet_upload_t u, upload_row_data_t **data);

static void uploads_gui_update_upload_info(gnet_upload_info_t *u);
static void uploads_gui_add_upload(gnet_upload_info_t *u);

/***
 *** Callbacks
 ***/

/**
 * Callback: called when an upload is removed from the backend.
 *
 * Either immediatly clears the upload from the frontend or just
 * set the upload_row_info->valid to FALSE, so we don't acidentially
 * try to use the handle to communicate with the backend.
 */
static void
upload_removed(gnet_upload_t uh, const gchar *reason,
    guint32 unused_running, guint32 unused_registered)
{
    gint row;
    upload_row_data_t *data;

	(void) unused_running;
	(void) unused_registered;

    /* Invalidate row and remove it from the gui if autoclear is on */
    row = find_row(uh, &data);
    if (row != -1) {
        GtkCList *clist =
            GTK_CLIST(gui_main_window_lookup("clist_uploads"));
        data->valid = FALSE;

        gtk_widget_set_sensitive(
            gui_main_window_lookup("button_uploads_clear_completed"),
            TRUE);

        if (reason != NULL)
            gtk_clist_set_text(clist, row, c_ul_status, reason);
    }
}

/**
 * Callback: called when an upload is added from the backend.
 *
 * Adds the upload to the gui.
 */
static void
upload_added(gnet_upload_t n, guint32 unused_running, guint32 unused_registered)
{
    gnet_upload_info_t *info;

	(void) unused_running;
	(void) unused_registered;

    info = guc_upload_get_info(n);
    uploads_gui_add_upload(info);
    guc_upload_free_info(info);
}

/**
 * Callback: called when upload information was changed by the backend.
 * This updates the upload information in the gui.
 */
static void
upload_info_changed(gnet_upload_t u,
	guint32 unused_running, guint32 unused_registered)
{
    gnet_upload_info_t *info;

	(void) unused_running;
	(void) unused_registered;

    info = guc_upload_get_info(u);
    uploads_gui_update_upload_info(info);
    guc_upload_free_info(info);
}

/***
 *** Private functions
 ***/

/**
 * Tries to fetch the row number and upload_row_data associated with a
 * given upload. The upload_row_data_t pointer data points to is only
 * updated when data != NULL and when the function returns a row number
 * != -1.
 */
static gint
find_row(gnet_upload_t u, upload_row_data_t **data)
{
    GtkCList *clist;
    GList *iter;
    upload_row_data_t fake;
    gint row = 0;

    fake.handle = u;
    fake.valid  = TRUE;

    clist = GTK_CLIST(gui_main_window_lookup("clist_uploads"));

    for (iter = clist->row_list; iter != NULL; iter = g_list_next(iter)) {
        GtkCListRow *r = iter->data;
		upload_row_data_t *rd;

		g_assert(r);
		rd = r->data;
		g_assert(rd);
		
        if (rd->valid && (rd->handle == u)) {
            /* found */

            if (data != NULL)
                *data = rd;
            return row;
        }

        row++;
    }

    g_warning("%s: upload not found [handle=%u]",
        G_GNUC_PRETTY_FUNCTION, u);

    return -1;
}

/**
 * Fetch the GUI row data associated with upload handle.
 */
upload_row_data_t *
uploads_gui_get_row_data(gnet_upload_t uhandle)
{
    upload_row_data_t *rd;

	return find_row(uhandle, &rd) < 0 ? NULL : rd;
}

static void
uploads_gui_update_upload_info(gnet_upload_info_t *u)
{
    gint row;
    GtkCList *clist_uploads;
    upload_row_data_t *rd;
	gnet_upload_status_t status;
	gchar size_tmp[256];
	gchar range_tmp[256];
	guint range_len;

    clist_uploads = GTK_CLIST(gui_main_window_lookup("clist_uploads"));
    row =  find_row(u->upload_handle, &rd);
	if (row == -1) {
        g_warning("%s: no matching row found [handle=%u]",
            G_GNUC_PRETTY_FUNCTION, u->upload_handle);
		return;
	}

	rd->range_start  = u->range_start;
	rd->range_end    = u->range_end;
	rd->start_date   = u->start_date;
	rd->last_update  = tm_time();

	if ((u->range_start == 0) && (u->range_end == 0)) {
		gtk_clist_set_text(clist_uploads, row, c_ul_size, "...");
		gtk_clist_set_text(clist_uploads, row, c_ul_range, "...");
	} else {
		g_strlcpy(size_tmp, short_size(u->file_size, show_metric_units()),
			sizeof size_tmp);

        range_len = gm_snprintf(range_tmp, sizeof range_tmp, "%s%s",
            short_size(u->range_end - u->range_start + 1,
				show_metric_units()),
			u->partial ? _(" (partial)") : "");

		if (u->range_start)
			range_len += gm_snprintf(
				&range_tmp[range_len], sizeof(range_tmp)-range_len,
					" @ %s", short_size(u->range_start, show_metric_units()));

		g_assert(range_len < sizeof(range_tmp));

		gtk_clist_set_text(clist_uploads, row, c_ul_size, size_tmp);
		gtk_clist_set_text(clist_uploads, row, c_ul_range, range_tmp);
	}

	gtk_clist_set_text(clist_uploads, row, c_ul_filename,
		(u->name != NULL) ? u->name : "...");
	gtk_clist_set_text(clist_uploads, row, c_ul_host,
			uploads_gui_host_string(u));
	gtk_clist_set_text(clist_uploads, row, c_ul_agent,
		(u->user_agent != NULL) ? lazy_utf8_to_locale(u->user_agent) : "...");

	guc_upload_get_status(u->upload_handle, &status);

	rd->status = status.status;

	if (u->push) {
		GdkColor *color = &(gtk_widget_get_style(GTK_WIDGET(clist_uploads))
			->fg[GTK_STATE_INSENSITIVE]);
		gtk_clist_set_foreground(clist_uploads, row, color);
	}

	gm_snprintf(range_tmp, sizeof range_tmp, "%5.02f%%",
		100.0 * uploads_gui_progress(&status, rd));
	gtk_clist_set_text(clist_uploads, row, c_ul_progress, range_tmp);

	gtk_clist_set_text(clist_uploads, row, c_ul_status,
		uploads_gui_status_str(&status, rd));
}


/**
 * Called to free the row data -- needed when running under -DTRACK_MALLOC.
 */
static void
free_data(gpointer o)
{
	wfree(o, sizeof(upload_row_data_t));
}

/**
 * Adds the given upload to the gui.
 */
void
uploads_gui_add_upload(gnet_upload_info_t *u)
{
 	gchar size_tmp[256];
	gchar range_tmp[256];
	guint range_len;
    gint row;
	const gchar *titles[UPLOADS_GUI_VISIBLE_COLUMNS];
    GtkWidget *clist_uploads;
    upload_row_data_t *data;

    clist_uploads = gui_main_window_lookup("clist_uploads");

	memset(titles, 0, sizeof(titles));

    if ((u->range_start == 0) && (u->range_end == 0)) {
        titles[c_ul_size] = titles[c_ul_range] =  "...";
    } else {
        g_strlcpy(size_tmp, short_size(u->file_size, show_metric_units()),
			sizeof size_tmp);

        range_len = gm_snprintf(range_tmp, sizeof range_tmp, "%s%s",
            short_size(u->range_end - u->range_start + 1,
				show_metric_units()),
			u->partial ? _(" (partial)") : "");

        if (u->range_start)
            range_len += gm_snprintf(
                &range_tmp[range_len], sizeof(range_tmp)-range_len,
                " @ %s", short_size(u->range_start, show_metric_units()));

        g_assert(range_len < sizeof(range_tmp));

        titles[c_ul_size]     = size_tmp;
        titles[c_ul_range]    = range_tmp;
    }

	titles[c_ul_filename] = u->name
							? lazy_utf8_to_ui_string(u->name)
							: "...";
	titles[c_ul_host]     = uploads_gui_host_string(u);
	titles[c_ul_loc]      = iso3166_country_cc(u->country);
    titles[c_ul_agent]    = (u->user_agent != NULL) ?
								lazy_utf8_to_locale(u->user_agent) : "...";
	titles[c_ul_progress]   = "...";
	titles[c_ul_status]   = "...";

    data = walloc0(sizeof *data);
    data->handle      = u->upload_handle;
    data->range_start = u->range_start;
    data->range_end   = u->range_end;
    data->start_date  = u->start_date;
    data->valid       = TRUE;
    data->gnet_addr   = zero_host_addr;
    data->gnet_port   = 0;

    row = gtk_clist_append(GTK_CLIST(clist_uploads),
			(gchar **) titles); /* override const */
    gtk_clist_set_row_data_full(GTK_CLIST(clist_uploads), row,
        data, free_data);
}

/***
 *** Public functions
 ***/

void
uploads_gui_early_init(void)
{
}

void
uploads_gui_init(void)
{
	GtkCList *clist;

	clist = GTK_CLIST(gui_main_window_lookup("clist_uploads"));
    gtk_clist_set_column_justification(clist, c_ul_size, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification(clist, c_ul_progress, GTK_JUSTIFY_RIGHT);
	gtk_clist_column_titles_passive(clist);

    guc_upload_add_upload_added_listener(upload_added);
    guc_upload_add_upload_removed_listener(upload_removed);
    guc_upload_add_upload_info_changed_listener(upload_info_changed);
}

/**
 * Unregister callbacks in the backend and clean up.
 */
void
uploads_gui_shutdown(void)
{
    guc_upload_remove_upload_added_listener(upload_added);
    guc_upload_remove_upload_removed_listener(upload_removed);
    guc_upload_remove_upload_info_changed_listener(upload_info_changed);
}

static gboolean
uploads_gui_is_visible(void)
{
	static GtkNotebook *notebook = NULL;
	gint current_page;

	if (!main_gui_window_visible())
		return FALSE;

	if (notebook == NULL)
		notebook = GTK_NOTEBOOK(gui_main_window_lookup("notebook_main"));

	current_page = gtk_notebook_get_current_page(notebook);

	return current_page == nb_main_page_uploads;
}

/**
 * Update all the uploads at the same time.
 */
void
uploads_gui_update_display(time_t now)
{
    static time_t last_update = 0;
	GtkCList *clist;
	GList *iter;
	gint row = 0;
    gnet_upload_status_t status;
    GSList *to_remove = NULL;
    GSList *sl;
	gboolean all_removed = TRUE;

	/*
	 * Usually don't perform updates if nobody is watching.  However,
	 * we do need to perform periodic cleanup of dead entries or the
	 * memory usage will grow.  Perform an update every UPDATE_MIN minutes
	 * at least.
	 *		--RAM, 28/12/2003
	 */

	if (!uploads_gui_is_visible() && now - last_update < UPDATE_MIN)
		return;

    if (last_update == now)
        return;

    last_update = now;

    clist = GTK_CLIST(gui_main_window_lookup("clist_uploads"));
    gtk_clist_freeze(clist);

	row = 0;
	for (iter = clist->row_list; iter; iter = g_list_next(iter), row++) {
        GtkCListRow *r = iter->data;
		upload_row_data_t *data;

		g_assert(r);
		data = r->data;
		g_assert(data);

        if (data->valid) {
			gchar tmp[20];

            data->last_update = now;
            guc_upload_get_status(data->handle, &status);

			gm_snprintf(tmp, sizeof tmp, "%5.02f%%",
				100.0 * uploads_gui_progress(&status, data));
			gtk_clist_set_text(clist, row, c_ul_progress, tmp);

            gtk_clist_set_text(clist, row, c_ul_status,
                uploads_gui_status_str(&status, data));
        } else {
            if (upload_should_remove(now, data))
                to_remove = g_slist_prepend(to_remove, GINT_TO_POINTER(row));
			else
				all_removed = FALSE;	/* Not removing all "expired" ones */
        }
    }

    for (sl = to_remove; sl != NULL; sl = g_slist_next(sl))
        gtk_clist_remove(clist, GPOINTER_TO_INT(sl->data));

    g_slist_free(to_remove);

    gtk_clist_thaw(clist);

	if (all_removed)
		gtk_widget_set_sensitive(
			gui_main_window_lookup("button_uploads_clear_completed"),
			FALSE);
}

static gboolean
uploads_clear_helper(gpointer unused_udata)
{
    GList *iter;
    GSList *to_remove= NULL;
    GSList *sl;
    guint row = 0;
    GtkCList *clist = GTK_CLIST(gui_main_window_lookup("clist_uploads"));

	(void) unused_udata;
    gtk_clist_freeze(clist);

    for (iter = clist->row_list; iter != NULL; iter = g_list_next(iter)) {
        GtkCListRow *r = iter->data;
		upload_row_data_t *rd;

		g_assert(r);
		rd = r->data;
		g_assert(rd);

        if (!rd->valid)
            to_remove = g_slist_prepend(to_remove, GINT_TO_POINTER(row));

        row++;
		if (row > uploads_rows_done) {
			uploads_rows_done++;
       		if (0 == (uploads_rows_done & 0x7f))
       			break;
		}
    }

    for (sl = to_remove; sl != NULL; sl = g_slist_next(sl))
        gtk_clist_remove(clist, GPOINTER_TO_INT(sl->data));

    g_slist_free(to_remove);
    gtk_clist_thaw(clist);

    if (iter == NULL) {
		gtk_widget_set_sensitive(
			gui_main_window_lookup("button_uploads_clear_completed"), FALSE);
    	uploads_remove_lock = FALSE;
    	return FALSE;
    }

    return TRUE;
}

void
uploads_gui_clear_completed(void)
{
	if (!uploads_remove_lock) {
		uploads_remove_lock = TRUE;
		uploads_rows_done = 0;
		gtk_timeout_add(100, uploads_clear_helper, NULL);
	}
}

/* vi: set ts=4 sw=4 cindent: */
