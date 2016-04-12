/* garu-playlist.c
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

#include "garu-tagger.h"
#include "garu-utils.h"
#include "garu-playlist.h"

struct _GaruPlaylist
{
  GtkBox       parent;

  GtkWidget   *tree_view;
  GtkWidget   *level_bar;
  GtkTreePath *path;
  GtkTreeIter  iter;
  
};

G_DEFINE_TYPE (GaruPlaylist, garu_playlist, GTK_TYPE_BOX);

enum {
  COLUMN_STATUS,
  COLUMN_TRACK,
  COLUMN_TITLE,
  COLUMN_ARTIST,
  COLUMN_ALBUM,
  COLUMN_GENRE,
  COLUMN_LENGTH,
  COLUMN_FILE,
  COLUMN_YEAR,
  COLUMN_MIMETYPE,
  NUM_COLUMNS
};

static const GtkTargetEntry targets[] =
  {
    { "REF_LIBRARY", GTK_TARGET_SAME_APP, TARGET_REF_LIBRARY }
  };

static void          garu_playlist_init_container     (GaruPlaylist *self);
static GtkListStore *garu_playlist_init_list_store    (void);
static void          garu_playlist_add_track          (GaruPlaylist           *self,
						       GtkTreePath            *path,
						       GtkTreeViewDropPosition pos,
						       gchar                  *uri);
static void          garu_playlist_add_track_dnd      (GPtrArray *data);

/* Signals */
static gboolean      garu_playlist_drag_drop          (GtkWidget      *widget,
						       GdkDragContext *context,
						       gint            x,
						       gint            y,
						       guint           time,
						       GaruPlaylist   *self);
static void          garu_playlist_drag_data_received (GtkWidget        *widget,
						       GdkDragContext   *context,
						       gint              x,
						       gint              y,
						       GtkSelectionData *data,
						       guint             info,
						       guint             time,
						       GaruPlaylist     *self);

static void
garu_playlist_dispose (GObject *object)
{
  GaruPlaylist *self = GARU_PLAYLIST (object);
  g_print ("DISPOSE\n");
  //g_free (self->file);
  //taglib_file_free (self->tfile);
  G_OBJECT_CLASS (garu_playlist_parent_class)->dispose (object);
}

static void
garu_playlist_finalize (GObject *object)
{
  GaruPlaylist *self = GARU_PLAYLIST (object);
  g_print ("Finaliza\n");
  //g_free (self->file);
  //taglib_file_free (self->tfile);
  G_OBJECT_CLASS (garu_playlist_parent_class)->finalize (object);
}

static void
garu_playlist_class_init (GaruPlaylistClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = garu_playlist_finalize;
  object_class->dispose = garu_playlist_dispose;
}

static void
garu_playlist_init (GaruPlaylist *self)
{
  garu_playlist_init_container (self);
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self),
				  GTK_ORIENTATION_VERTICAL);
}

static void
garu_playlist_init_container (GaruPlaylist *self)
{
  GtkWidget         *tree_view, *image, *scrolled, *level_bar;
  GtkListStore      *store;
  GtkTreeSelection  *selection;
  GtkTreeViewColumn *column;
  GtkCellRenderer   *renderer;
  GtkStyleContext   *context;

  store = garu_playlist_init_list_store ();
  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (tree_view), TRUE);
  
  gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW(tree_view),
  					  GDK_BUTTON1_MASK,
  					  targets,
  					  G_N_ELEMENTS(targets),
  					  GDK_ACTION_COPY | GDK_ACTION_MOVE);

  gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (tree_view),
  					targets,
  					G_N_ELEMENTS(targets),
  					GDK_ACTION_COPY | GDK_ACTION_MOVE);
  /* g_signal_connect (tree_view, "drag-drop", */
  /* 		    G_CALLBACK (garu_playlist_drag_drop), NULL); */
  g_signal_connect (tree_view, "drag-data-received",
  		    G_CALLBACK (garu_playlist_drag_data_received), self);

  /* STATUS */
  image = gtk_image_new_from_icon_name ("audio-volume-high-symbolic",
                                        GTK_ICON_SIZE_MENU);
  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes(NULL,
						    renderer,
						    "icon-name",
						    COLUMN_STATUS,
						    NULL);
  gtk_tree_view_column_set_widget (column, image);
  gtk_widget_show (image);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_set_reorderable (column, TRUE);
  gtk_tree_view_column_set_visible (column, TRUE);
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (column, 36);
  /* TRACK */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes(_("Track"),
						    renderer,
						    "text",
						    COLUMN_TRACK,
						    NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_set_reorderable (column, TRUE);
  /* TITLE */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes(_("Title"),
						    renderer,
						    "text",
						    COLUMN_TITLE,
						    NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_set_resizable (column, TRUE);
  gtk_tree_view_column_set_reorderable (column, TRUE);
  /* ARTIST */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes(_("Artist"),
						    renderer,
						    "text",
						    COLUMN_ARTIST,
						    NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_set_resizable (column, TRUE);
  gtk_tree_view_column_set_reorderable (column, TRUE);
  /* ALBUM */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes(_("Album"),
						    renderer,
						    "text",
						    COLUMN_ALBUM,
						    NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_set_resizable (column, TRUE);
  gtk_tree_view_column_set_reorderable (column, TRUE);
  /* GENRE */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes(_("Genere"),
						    renderer,
						    "text",
						    COLUMN_GENRE,
						    NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_set_resizable (column, TRUE);
  gtk_tree_view_column_set_reorderable (column, TRUE);
  /* LENGTH */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes(_("Length"),
						    renderer,
						    "text",
						    COLUMN_LENGTH,
						    NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_set_resizable (column, TRUE);
  gtk_tree_view_column_set_reorderable (column, TRUE);
  /* FILENAME */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes(_("File"),
						    renderer,
						    "text",
						    COLUMN_FILE,
						    NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_set_resizable (column, TRUE);
  gtk_tree_view_column_set_reorderable (column, TRUE);
  /* YEAR */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes(_("Year"),
						    renderer,
						    "text",
						    COLUMN_YEAR,
						    NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_set_resizable (column, TRUE);
  gtk_tree_view_column_set_reorderable (column, TRUE);
  /* MIMETYPE */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes(_("Mimetype"),
						    renderer,
						    "text",
						    COLUMN_MIMETYPE,
						    NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_set_resizable (column, TRUE);
  gtk_tree_view_column_set_reorderable (column, TRUE);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrolled), tree_view);
  gtk_widget_show_all (scrolled);
  gtk_box_pack_start (GTK_BOX (self), scrolled, TRUE, TRUE, 0);

  level_bar = gtk_level_bar_new ();
  gtk_level_bar_set_value (GTK_LEVEL_BAR (level_bar), 0.1);
  gtk_box_pack_start (GTK_BOX (self), level_bar, FALSE, FALSE, 0);

  self->tree_view = tree_view;
  self->level_bar = level_bar;
}

static GtkListStore *
garu_playlist_init_list_store (void)
{
  GtkListStore *store;
  store = gtk_list_store_new (NUM_COLUMNS,
                              G_TYPE_STRING, /* STATUS */
                              G_TYPE_STRING, /* TRACK */
                              G_TYPE_STRING, /* TITLE */
                              G_TYPE_STRING, /* ARTIST */
                              G_TYPE_STRING, /* ALBUM */
                              G_TYPE_STRING, /* GENRE */
                              G_TYPE_STRING, /* LENGTH */
                              G_TYPE_STRING, /* FILENAME */
			      G_TYPE_STRING, /* COLUMN_YEAR */
                              G_TYPE_STRING  /* MIMETYPE */ );
  return store;
}

/* Signals */
static void
garu_folder_browser_drag_data_get (GtkWidget        *widget,
				   GdkDragContext   *context,
				   GtkSelectionData *data,
				   guint             info,
				   guint             time,
				   GaruPlaylist     *self)
{
  g_print ("envia get\n");
  
}

static gboolean
garu_playlist_drag_drop (GtkWidget      *widget,
			 GdkDragContext *context,
			 gint            x,
			 gint            y,
			 guint           time,
			 GaruPlaylist   *self)
{
  GList *p;
  g_print ("Vino\n");
  if (gdk_drag_context_list_targets (context) == NULL)
    return FALSE;

  for (p = gdk_drag_context_list_targets (context); p != NULL; p = p->next) {
    gchar *possible_type;
    g_print ("Llrgo\n");
    possible_type = gdk_atom_name (GDK_POINTER_TO_ATOM (p->data));
    if (!strcmp (possible_type, "REF_LIBRARY")) {

      gtk_drag_get_data(widget,
			context,
			GDK_POINTER_TO_ATOM (p->data),
			time);

      g_free (possible_type);

      return TRUE;
    }
    g_print ("Paso\n");
    g_free (possible_type);
  }

  return FALSE;
}

static void
garu_playlist_add_track_dnd (GPtrArray *data)
{
  GSList                  *l;
  GaruPlaylist            *self = g_ptr_array_index (data, 0);
  GtkTreePath             *path = g_ptr_array_index (data, 1);
  GtkTreeViewDropPosition *pos = g_ptr_array_index (data, 2);
  GSList                  *list = g_ptr_array_index (data, 3);

  for (l = list; l != NULL; l = l->next)
      garu_playlist_add_track (self, path, *pos, l->data);

  gtk_tree_path_free (path);
  g_ptr_array_free(data, TRUE);
  g_slist_free_full (list, (GDestroyNotify) g_free);
}

static void
garu_playlist_add_track (GaruPlaylist           *self,
			 GtkTreePath            *path,
			 GtkTreeViewDropPosition pos,
			 gchar                  *uri)
{
  GaruTagger   *tagger;
  GtkTreeModel *model;
  GtkTreeIter   sibling, iter;
  gchar        *track, *title, *artist, *album, *genre, *length, *year;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->tree_view));

  tagger = garu_tagger_new ();
  if (!garu_tagger_set_uri (tagger, uri))
    {
      g_object_unref (tagger);
      return;
    }

  if (path != NULL)
    {
      gtk_tree_model_get_iter (model, &sibling, path);
      gtk_tree_path_next (path);
      switch (pos)
	{
	case GTK_TREE_VIEW_DROP_AFTER:
	case GTK_TREE_VIEW_DROP_INTO_OR_AFTER:
	  gtk_list_store_insert_after (GTK_LIST_STORE (model),
				       &iter, &sibling);
	  break;
	case GTK_TREE_VIEW_DROP_BEFORE:
	case GTK_TREE_VIEW_DROP_INTO_OR_BEFORE:
	  gtk_list_store_insert_before (GTK_LIST_STORE (model),
					&iter, &sibling);
	  break;
	}
    }
  else
    {
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    }

  track = garu_tagger_get_track_str (tagger);
  title = garu_tagger_get_title (tagger);
  artist = garu_tagger_get_artist (tagger);
  album = garu_tagger_get_album (tagger);
  genre = garu_tagger_get_genre (tagger);
  length = garu_tagger_get_length_str (tagger);
  year = garu_tagger_get_year_str (tagger);

  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      COLUMN_TRACK, track,
		      COLUMN_TITLE, title,
		      COLUMN_ARTIST, artist,
		      COLUMN_ALBUM, album,
		      COLUMN_GENRE, genre,
		      COLUMN_LENGTH, length,
		      COLUMN_FILE, garu_tagger_get_file (tagger),
		      COLUMN_YEAR, year, -1);
  g_free (track);
  g_free (title);
  g_free (artist);
  g_free (album);
  g_free (genre);
  g_free (length);
  g_free (year);
  g_object_unref (tagger);
}
static void
garu_playlist_add_song (GaruPlaylist *self, GaruTagger *tagger)
{
  GtkTreeIter   iter;
  GtkTreeModel *model;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->tree_view));

  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
}

static void
garu_playlist_drag_data_received (GtkWidget        *widget,
				  GdkDragContext   *context,
				  gint              x,
				  gint              y,
				  GtkSelectionData *data,
				  guint             info,
				  guint             time,
				  GaruPlaylist     *self)
{
  gint                     i;
  gchar                  **uris;
  GtkTreeView             *tree_view;
  GtkTreePath             *path;
  GtkTreeViewDropPosition  pos;
  GSList                  *list = NULL;
  GThread                 *thread;
  GPtrArray               *array;

  uris = g_uri_list_extract_uris (gtk_selection_data_get_data (data));
  if (uris[0] == NULL)
    return;
  for (i = 0; uris[i] != NULL; i++)
    list = g_slist_append (list, g_strdup (uris[i]));

  tree_view = GTK_TREE_VIEW (widget);
  gtk_tree_view_get_dest_row_at_pos (tree_view, x, y, &path, &pos);

  /* Array */
  array = g_ptr_array_new ();
  g_ptr_array_add (array, self);
  g_ptr_array_add (array, path);
  g_ptr_array_add (array, &pos);
  g_ptr_array_add (array, list);

  thread = g_thread_new ("add-track",
			 (GThreadFunc) garu_playlist_add_track_dnd, array);

  /* Free memory */
  g_strfreev(uris);
  gtk_drag_finish (context, TRUE, FALSE, time);
}

GaruPlaylist *
garu_playlist_new (void)
{
  return g_object_new (GARU_TYPE_PLAYLIST, NULL);
}

gchar *
garu_playlist_get_track (GaruPlaylist *self)
{
  gchar            *file;
  GList            *list;
  GtkTreePath      *path;
  GtkTreeModel     *model;
  GtkTreeSelection *selection;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->tree_view));

  /* If the playlist is empty */
  if (!gtk_tree_model_get_iter_first (model, &self->iter))
    return NULL;
    
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->tree_view));
  list = gtk_tree_selection_get_selected_rows (selection, &model);

  if (list != NULL)
      gtk_tree_model_get_iter (model, &self->iter, list->data);

  gtk_tree_model_get (model, &self->iter, COLUMN_FILE, &file, -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &self->iter,
		      COLUMN_STATUS, "media-playback-start-symbolic", -1);
  self->path = gtk_tree_model_get_path (model, &self->iter);

  g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free);
  return file;
}

gchar *
garu_playlist_get_next_track (GaruPlaylist *self)
{
  GtkTreeIter iter;
  gchar *file;
  GtkTreeModel     *model;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->tree_view));

  gtk_tree_model_get_iter (model, &iter, self->path);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_STATUS, NULL, -1);

  if (!gtk_tree_model_iter_next (model, &iter))
    return NULL;

  if (self->path == NULL)
    g_print ("Es null");
  else
    g_print ("No es null");

  gtk_tree_model_get (model, &iter, COLUMN_FILE, &file, -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      COLUMN_STATUS, "media-playback-start-symbolic", -1);
  self->path = gtk_tree_model_get_path (model, &iter);
  return file;
}
