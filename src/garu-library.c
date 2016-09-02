/* garu-library.c
 *
 * Copyright (C) 2016 Michael Yugcha <mgyugcha@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "garu-library.h"
#include "garu-folder-browser.h"
#include "garu-utils.h"

struct _GaruLibrary
{
  GtkBox parent;
};

G_DEFINE_TYPE (GaruLibrary, garu_library, GTK_TYPE_BOX)

/*
 * the view options shows the available options on Garu Music
 * Player. This options are: folders, artists, albums, genres,
 * radios.
 */
enum
{
  OPTION_FOLDERS,
  OPTION_ARTISTS,
  OPTION_ALBUMS,
  OPTION_GENRES,
  OPTION_RADIOS
};

enum
{
  COLUMN_ID,
  COLUMN_STRING,
  COLUMN_PIXBUF,
  NUM_COLUMNS
};

static void          garu_library_init_container    (GaruLibrary       *self);
static GtkWidget    *garu_library_init_view_options (GaruLibrary       *self);
static GtkListStore *garu_library_get_options       (GaruLibrary       *self);
static void          garu_library_row_actived       (GtkTreeView       *tree_view,
						     GtkTreePath       *path,
						     GtkTreeViewColumn *column);

static void
garu_library_init (GaruLibrary *self)
{
  garu_library_init_container (self);
}

static void
garu_library_class_init (GaruLibraryClass *klass)
{
}

static void
garu_library_init_container (GaruLibrary *self)
{
  GtkWidget *folder_structure, *separator, *view_options;

  view_options = garu_library_init_view_options (self);
  gtk_box_pack_start (GTK_BOX (self), view_options, FALSE, FALSE, 0);

  separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (self), separator, FALSE, FALSE, 0);  

  folder_structure = garu_folder_browser_new ();
  gtk_box_pack_start (GTK_BOX (self), folder_structure, TRUE, TRUE, 0);

}

/*
 * the view options shows the available options on Garu Music
 * Player. This options are: folders, artists, albums, genres,
 * radios.
 */
static GtkWidget *
garu_library_init_view_options (GaruLibrary *self)
{
  GtkWidget         *tree_view;
  GtkListStore      *store;
  GtkTreeViewColumn *column;
  GtkCellRenderer   *renderer;
  GtkStyleContext   *context;

  store = garu_library_get_options (self);

  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);
  context = gtk_widget_get_style_context (tree_view);
  gtk_style_context_add_class (context, "sidebar");

  column = gtk_tree_view_column_new ();

  /* text */
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_renderer_set_padding (renderer,
                                 GARU_CELL_RENDERER_PADDING,
                                 GARU_CELL_RENDERER_PADDING);
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_add_attribute (column, renderer,
                                      "text", COLUMN_STRING);
  /* icon */
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_renderer_set_padding (renderer,
                                 GARU_CELL_RENDERER_PADDING,
                                 GARU_CELL_RENDERER_PADDING);
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer,
                                      "icon-name", COLUMN_PIXBUF);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_set_activate_on_single_click (GTK_TREE_VIEW (tree_view), TRUE);

  /* signals */
  g_signal_connect (tree_view, "row-activated",
		    G_CALLBACK (garu_library_row_actived), NULL);

  return tree_view;
}

static GtkListStore *
garu_library_get_options (GaruLibrary *self)
{
  GtkListStore *store;
  GtkTreeIter   iter;

  store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_INT,
			      G_TYPE_STRING, G_TYPE_STRING);
  /* folders */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COLUMN_ID, OPTION_FOLDERS,
                      COLUMN_STRING, _("Folders"),
                      COLUMN_PIXBUF, "inode-directory-symbolic", -1);
  /* artist */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COLUMN_ID, OPTION_ARTISTS,
                      COLUMN_STRING, _("Artist"),
                      COLUMN_PIXBUF, "system-users-symbolic", -1);
  /* albums */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COLUMN_ID, OPTION_ALBUMS,
                      COLUMN_STRING, _("Albums"),
                      COLUMN_PIXBUF, "media-optical-cd-audio-symbolic", -1);
  /* genres */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COLUMN_ID, OPTION_GENRES,
                      COLUMN_STRING, _("Genres"),
                      COLUMN_PIXBUF, "system-file-manager-symbolic", -1);
  /* radios */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COLUMN_ID, OPTION_RADIOS,
                      COLUMN_STRING, _("Radios"),
                      COLUMN_PIXBUF, "audio-input-microphone-symbolic", -1);
  return store;
}

static void
garu_library_row_actived (GtkTreeView       *tree_view,
			  GtkTreePath       *path,
			  GtkTreeViewColumn *column)
{
  GSettings *settings;
  gchar     *position;

  settings = garu_utils_get_settings ();
  position = gtk_tree_path_to_string (path);
  g_settings_set_string (settings, "library-option-position", position);
  g_print (g_settings_get_string (settings, "library-option-position"));
  g_free (position);
}

GaruLibrary *
garu_library_new (void)
{
  return g_object_new (GARU_TYPE_LIBRARY,
                       "orientation", GTK_ORIENTATION_VERTICAL,
                       NULL);
}
