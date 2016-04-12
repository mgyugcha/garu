/* garu-folder-browser.c
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

#include "garu-application.h"
#include "garu-folder-browser.h"
#include "garu-utils.h"

struct _GaruFolderBrowser
{
  GtkScrolledWindow parent;

  GtkWidget        *tree_view;
};

G_DEFINE_TYPE (GaruFolderBrowser, garu_folder_browser, GTK_TYPE_SCROLLED_WINDOW);

enum {
  FILE_TYPE_DIR,
  FILE_TYPE_SONG
};

enum {
  COLUMN_ICON,
  COLUMN_NAME,
  COLUMN_FILENAME,
  COLUMN_TYPE,
  NUM_COLUMNS
};

static const GtkTargetEntry targets[] =
  {
    { "REF_LIBRARY", GTK_TARGET_SAME_APP, TARGET_REF_LIBRARY }
  };

static GSList       *garu_folder_browser_get_files       (gchar *directory,
							  gint type);
static GSList       *garu_folder_browser_get_subfiles    (gchar *directory);
static GtkListStore *garu_folder_browser_init_list_store ();
static void          garu_folder_browser_init_container  (GaruFolderBrowser *self);
static void          garu_folder_browser_load_directory  (GaruFolderBrowser *self,
							  gchar *directory);

/* Signals */
static gboolean      garu_folder_browser_button_press    (GtkWidget *widget,
							  GdkEventButton *event,
							  GaruFolderBrowser *self);
static void          garu_folder_browser_drag_data_get   (GtkWidget         *widget,
							  GdkDragContext    *context,
							  GtkSelectionData  *data,
							  guint              info,
							  guint              time);


static void
garu_folder_browser_finalize (GObject *object)
{
  G_OBJECT_CLASS (garu_folder_browser_parent_class)->finalize (object);
}

static void
garu_folder_browser_class_init (GaruFolderBrowserClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = garu_folder_browser_finalize;
}

static void
garu_folder_browser_init (GaruFolderBrowser *self)
{
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  garu_folder_browser_init_container (self);
  garu_folder_browser_load_directory (self, (gchar*)g_get_home_dir ());
}

static void
garu_folder_browser_init_container (GaruFolderBrowser *self)
{
  GtkWidget         *tree_view;
  GtkTreeViewColumn *column;
  GtkTreeSelection  *selection;
  GtkListStore      *liststore;
  GtkCellRenderer   *renderer;
  GtkStyleContext   *context;

  liststore = garu_folder_browser_init_list_store (self);
  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (liststore));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  context = gtk_widget_get_style_context (tree_view);
  gtk_style_context_add_class (context, "sidebar");

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_min_width (column, 160);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  /* Icon */
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_renderer_set_padding (renderer,
                                 GARU_CELL_PADDING,
                                 GARU_CELL_PADDING);
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer,
				      "icon-name", COLUMN_ICON);
  /* File's name */
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_renderer_set_padding (renderer,
				 GARU_CELL_PADDING, GARU_CELL_PADDING);
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_add_attribute (column, renderer, "text", COLUMN_NAME);
  g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

  gtk_container_add (GTK_CONTAINER (self), tree_view);

  /* Connect signals */
  g_signal_connect (tree_view, "button-press-event",
                    G_CALLBACK (garu_folder_browser_button_press), self);
  g_signal_connect (tree_view, "drag-data-get",
                    G_CALLBACK (garu_folder_browser_drag_data_get), NULL);

  /* Drag */
  gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW(tree_view),
                                          GDK_BUTTON1_MASK,
                                          targets,
                                          G_N_ELEMENTS(targets),
                                          GDK_ACTION_COPY);
  self->tree_view = tree_view;
}

static GtkListStore *
garu_folder_browser_init_list_store ()
{
  GtkListStore *liststore;
  liststore = gtk_list_store_new (NUM_COLUMNS,
				  G_TYPE_STRING, /* COLUMN_ICON */
				  G_TYPE_STRING, /* COLUMN_NAME */
				  G_TYPE_STRING, /* COLUMN_FILENAME */
				  G_TYPE_INT);	 /* COLUMN_TYPE */
  return liststore;
}

static void
garu_folder_browser_load_directory (GaruFolderBrowser *self, gchar *directory)
{
  GtkTreeIter   iter;
  GSList       *folders, *files, *l;
  GtkListStore *liststore;
  GtkTreeView  *tree_view;
  gchar        *filename, *path_filename;

  tree_view = GTK_TREE_VIEW (self->tree_view);
  liststore = GTK_LIST_STORE (gtk_tree_view_get_model (tree_view));
  gtk_list_store_clear (liststore);

  folders = garu_folder_browser_get_files (directory, FILE_TYPE_DIR);
  files = garu_folder_browser_get_files (directory, FILE_TYPE_SONG);

  /* Back button */
  if (g_strcmp0 (directory, g_get_home_dir ()) != 0)
    {
      gchar *filename;
      filename = g_path_get_basename (directory);
      path_filename = g_path_get_dirname (directory);
      gtk_list_store_prepend (liststore, &iter);
      gtk_list_store_set (liststore, &iter,
                          COLUMN_ICON, "go-previous-symbolic",
                          COLUMN_NAME, filename,
                          COLUMN_FILENAME, path_filename, -1);
      g_free (filename);
      g_free (path_filename);
    }

  /* Add folders */
  for (l = folders; l != NULL; l = l->next)
    {
      filename = g_path_get_basename (l->data);
      gtk_list_store_append (liststore, &iter);
      gtk_list_store_set (liststore, &iter,
                          COLUMN_ICON, "folder-symbolic",
                          COLUMN_NAME, filename,
                          COLUMN_FILENAME, l->data,
      			  COLUMN_TYPE, FILE_TYPE_DIR, -1);
      g_free (filename);
    }

  /* Add files */
  for (l = files; l != NULL; l = l->next)
    {
      filename = g_path_get_basename (l->data);
      gtk_list_store_append (liststore, &iter);
      gtk_list_store_set (liststore, &iter,
                          COLUMN_ICON, "emblem-music-symbolic",
                          COLUMN_NAME, filename,
                          COLUMN_FILENAME, l->data,
  			  COLUMN_TYPE, FILE_TYPE_SONG, -1);
      g_free (filename);
    }

  g_slist_free_full (files, (GDestroyNotify) g_free);
  g_slist_free_full (folders, (GDestroyNotify) g_free);
}

static GSList *
garu_folder_browser_get_files (gchar *directory, gint type)
{
  GDir        *dir;
  gchar       *path_filename;
  GSList      *list = NULL;
  const gchar *filename = NULL;

  dir = g_dir_open (directory, 0, NULL);
  if (dir == NULL)
    return NULL;

  filename = g_dir_read_name (dir);
  while (filename)
    {
      path_filename = g_build_filename (directory, filename, NULL);
      switch (type)
        {
        case FILE_TYPE_DIR:
          if (g_file_test (path_filename, G_FILE_TEST_IS_DIR)
              && !garu_utils_is_hidden (path_filename))
	    list = g_slist_prepend (list, path_filename);
          break;
        case FILE_TYPE_SONG:
          if (g_file_test (path_filename, G_FILE_TEST_IS_REGULAR)
              && !garu_utils_is_hidden (path_filename)
              && garu_utils_is_audio (path_filename))
	    list = g_slist_prepend (list, path_filename);
          break;
        }
      filename = g_dir_read_name (dir);
    }
  list = g_slist_sort (list, (GCompareFunc) g_strcmp0);
  g_dir_close (dir);
  return list;
}

static GSList *
garu_folder_browser_get_subfiles (gchar *directory)
{
  GSList *subdirs, *subsongs, *l;

  subdirs = garu_folder_browser_get_files (directory, FILE_TYPE_DIR);
  subsongs = garu_folder_browser_get_files (directory, FILE_TYPE_SONG);

  for (l = subdirs; l != NULL; l = l->next)
    {
      GSList *subsubsongs;
      subsubsongs = garu_folder_browser_get_subfiles (l->data);
      if (subsubsongs != NULL)
	subsongs = g_slist_concat (subsongs, subsubsongs);
    }

  g_slist_free_full (subdirs, (GDestroyNotify) g_free);
  return subsongs;
}

static gboolean
garu_folder_browser_button_press (GtkWidget *widget,
				  GdkEventButton *event,
				  GaruFolderBrowser *self)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (widget);
  GtkTreePath *path;
  GtkTreeIter iter;
  GtkTreeModel *tree_model;
  gchar *type, *path_filename, *uri;
  GaruApplication *app;
  GaruPlayer *player;
  GError *error = NULL;

  if ((event->type == GDK_DOUBLE_BUTTON_PRESS)
      && (event->button == GDK_BUTTON_PRIMARY))
    {
      tree_model = gtk_tree_view_get_model (tree_view);
      gtk_tree_view_get_cursor (tree_view, &path, NULL);
      gtk_tree_model_get_iter (tree_model, &iter, path);
      gtk_tree_model_get (tree_model, &iter,
                          COLUMN_ICON, &type,
                          COLUMN_FILENAME, &path_filename, -1) ;

      if (g_file_test (path_filename, G_FILE_TEST_IS_DIR))
	{
	  g_print ("%s\n", path_filename);
	  garu_folder_browser_load_directory (self, path_filename);
	}
      else
        {
          app = GARU_APPLICATION (g_application_get_default ());
          player = garu_application_get_player (app);
          uri = g_filename_to_uri (path_filename, NULL, &error);
          garu_player_set_track (player, uri);
          garu_player_play (player);
        }

      g_free (type);
      gtk_tree_path_free (path);
    }
  return FALSE;
}

/* Signals */
static void
garu_folder_browser_drag_data_get (GtkWidget        *widget,
                                   GdkDragContext   *context,
                                   GtkSelectionData *data,
                                   guint             info,
                                   guint             time)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GString          *string;
  GtkTreeIter       iter;
  gchar            *uri, *addr;
  const gchar      *address;
  GList            *list = NULL, *l;
  GSList           *songs, *m;

  switch (info)
    {
    case TARGET_REF_LIBRARY:
      string = g_string_new (NULL);
      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
      list = gtk_tree_selection_get_selected_rows (selection, &model);

      for (l = list; l != NULL; l = l->next)
	{
	  GValue type = G_VALUE_INIT;
	  GValue dir = G_VALUE_INIT;
	  gtk_tree_model_get_iter (model, &iter, l->data);
	  gtk_tree_model_get_value (model, &iter, COLUMN_TYPE, &type);
	  gtk_tree_model_get_value (model, &iter, COLUMN_FILENAME, &dir);
	  address = g_value_get_string (&dir);

	  switch (g_value_get_int (&type))
	    {
	    case FILE_TYPE_DIR:
	      addr = g_strdup (address);
	      songs = garu_folder_browser_get_subfiles (addr);
	      g_free (addr);
	      for (m = songs; m != NULL; m = m->next)
		{
		  uri = g_filename_to_uri (m->data, NULL, NULL);
		  g_string_append (string, uri);
		  g_string_append (string, "\r\n");
		  g_free (uri);
		}
	      g_slist_free_full (songs, (GDestroyNotify) g_free);
	      break;
	    case FILE_TYPE_SONG:
	      uri = g_filename_to_uri (address, NULL, NULL);
	      g_string_append (string, uri);
	      g_string_append (string, "\r\n");
	      g_free (uri);
	      break;
	    }
	  g_value_unset (&type);
	  g_value_unset (&dir);
	}
      gtk_selection_data_set (data, gtk_selection_data_get_target(data),
      			      8, string->str, string->len);
      g_string_free (string, TRUE);
      g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free);
      break;
    default:
      g_warning ("Some error here?");
      break;
    }
}


GtkWidget *
garu_folder_browser_new (void)
{
  return g_object_new (GARU_TYPE_FOLDER_BROWSER, NULL);
}
