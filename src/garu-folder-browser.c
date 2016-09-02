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
  gchar            *directory;
};

G_DEFINE_TYPE (GaruFolderBrowser, garu_folder_browser, GTK_TYPE_BOX);

/**
 * For GaruFolderBroser there are only 2 types of files: directories
 * and music files.
 */
enum {
  FILE_TYPE_DIR,
  FILE_TYPE_MUSIC
};

/**
 * The GtkTreeView only can show icon (folder or music file) and the
 * filename, the file path and the file type are hidden.
 */
enum {
  COLUMN_ICON,
  COLUMN_FILENAME,
  COLUMN_FILE_PATH,
  COLUMN_TYPE,
  NUM_COLUMNS
};

/* properties */
enum {
  PROP_0,
  PROP_DIRECTORY,
  LAST_PROP
};
static GParamSpec *properties [LAST_PROP];

/* targets for DND */
static const GtkTargetEntry targets[] =
  {
    { "FILE_PATH", GTK_TARGET_SAME_APP, TARGET_FILE_PATH }
  };

static GSList       *garu_folder_browser_get_files              (gchar             *directory,
                                                                 gint               type);
static GSList       *garu_folder_browser_get_all_music_files    (gchar             *directory);
static GtkListStore *garu_folder_browser_init_list_store        (void);
static void          garu_folder_browser_init_container         (GaruFolderBrowser *self);
static GtkWidget    *garu_folder_browser_init_action_buttons    (GaruFolderBrowser *self);
static void          garu_folder_browser_load_directory         (GaruFolderBrowser *self);
static void          garu_folder_browser_load_settings          (GaruFolderBrowser *self);
/* getters and setters */
static void          garu_folder_browser_set_directory          (GaruFolderBrowser *self,
                                                                 const gchar       *directory);
static const gchar  *garu_folder_browser_get_directory          (GaruFolderBrowser *self);
/* signals */
static void          garu_folder_browser_clicked_home_button    (GtkButton         *button,
                                                                 GaruFolderBrowser *self);
static void          garu_folder_browser_clicked_browser_button (GtkButton         *button,
                                                                 GaruFolderBrowser *self);
static gboolean      garu_folder_browser_button_press           (GtkWidget         *widget,
                                                                 GdkEventButton    *event,
                                                                 GaruFolderBrowser *self);
static void          garu_folder_browser_drag_data_get          (GtkWidget         *widget,
                                                                 GdkDragContext    *context,
                                                                 GtkSelectionData  *data,
                                                                 guint              info,
                                                                 guint              time);

static void
garu_folder_browser_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  GaruFolderBrowser *self = GARU_FOLDER_BROWSER (object);
  switch (property_id)
    {
    case PROP_DIRECTORY:
      g_value_set_string (value,
                          garu_folder_browser_get_directory (self));
      break;
    }
}

static void
garu_folder_browser_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  GaruFolderBrowser *self = GARU_FOLDER_BROWSER (object);

  switch (property_id)
    {
    case PROP_DIRECTORY:
      garu_folder_browser_set_directory (self,
                                         g_value_get_string (value));
      break;
    }
}

static void
garu_folder_browser_finalize (GObject *object)
{
  G_OBJECT_CLASS (garu_folder_browser_parent_class)->finalize (object);
}

static void
garu_folder_browser_class_init (GaruFolderBrowserClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = garu_folder_browser_get_property;
  object_class->set_property = garu_folder_browser_set_property;
  object_class->finalize = garu_folder_browser_finalize;

  /* properties */
  properties [PROP_DIRECTORY] =
    g_param_spec_string ("directory",
                         "Directory",
                         "The directory to which the user accesses",
                         NULL,
                         G_PARAM_READWRITE);
  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
garu_folder_browser_init (GaruFolderBrowser *self)
{
  garu_folder_browser_init_container (self);

  /* GaruFolderBroswer shows the directory when the settings are
     loaded. */
  garu_folder_browser_load_settings (self);
}
static void
garu_folder_browser_load_settings (GaruFolderBrowser *self)
{
  GSettings *settings;
  settings = garu_utils_get_settings ();
  g_settings_bind (settings, "folder-browser-directory",
                   self, "directory", G_SETTINGS_BIND_DEFAULT);
}

static void
garu_folder_browser_init_container (GaruFolderBrowser *self)
{
  GtkWidget         *tree_view, *scrolled_window, *button_box;
  GtkTreeViewColumn *column;
  GtkTreeSelection  *selection;
  GtkListStore      *liststore;
  GtkCellRenderer   *renderer;
  GtkStyleContext   *context;

  liststore = garu_folder_browser_init_list_store (self);
  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (liststore))
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);
  /* drag and drop */
  /* gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW(tree_view), */
  /*                                         GDK_BUTTON1_MASK, */
  /*                                         targets, */
  /*                                         G_N_ELEMENTS(targets), */
  /*                                         GDK_ACTION_COPY); */
  g_object_unref (liststore);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(tree_view));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  context = gtk_widget_get_style_context (tree_view);
  gtk_style_context_add_class (context, "sidebar");

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_min_width (column, 160);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  /* icon */
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_renderer_set_padding (renderer,
                                 GARU_CELL_RENDERER_PADDING,
                                 GARU_CELL_RENDERER_PADDING);
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer,
                                      "icon-name", COLUMN_ICON);
  /* filename */
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_renderer_set_padding (renderer,
                                 GARU_CELL_RENDERER_PADDING,
                                 GARU_CELL_RENDERER_PADDING);
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_add_attribute (column, renderer,
                                      "text", COLUMN_FILENAME);
  g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

  /* box container */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
  button_box = garu_folder_browser_init_action_buttons (self);

  gtk_box_pack_start (GTK_BOX (self), scrolled_window, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (self), button_box, FALSE, FALSE, 0);

  /* Connect signals */
  g_signal_connect (tree_view, "button-press-event",
                    G_CALLBACK (garu_folder_browser_button_press), self);
  /* g_signal_connect (tree_view, "drag-data-get", */
  /*                   G_CALLBACK (garu_folder_browser_drag_data_get), NULL); */
  self->tree_view = tree_view;
}

static GtkWidget *
garu_folder_browser_init_action_buttons (GaruFolderBrowser *self)
{
  GtkWidget *box;
  GtkWidget *button, *image;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_set_border_width (GTK_CONTAINER (box), 4);

  /* add button */
  button = garu_utils_new_icon_button ("list-add-symbolic", FALSE, FALSE);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  /* home button */
  button = garu_utils_new_icon_button ("go-home-symbolic", FALSE, FALSE);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (garu_folder_browser_clicked_home_button),
                    self);
  /* browse button */
  button = garu_utils_new_icon_button ("folder-symbolic", FALSE, FALSE);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (garu_folder_browser_clicked_browser_button),
                    self);

  return box;
}

static GtkListStore *
garu_folder_browser_init_list_store (void)
{
  GtkListStore *liststore;
  liststore = gtk_list_store_new (NUM_COLUMNS,
                                  G_TYPE_STRING, /* COLUMN_ICON */
                                  G_TYPE_STRING, /* COLUMN_FILENAME */
                                  G_TYPE_STRING, /* COLUMN_FILE_PATH */
                                  G_TYPE_INT);   /* COLUMN_TYPE */
  return liststore;
}

/**
 * Display the directory in the GtkTreeView. If the 'directory' is
 * NULL then shows the home directory.
 */
static void
garu_folder_browser_load_directory (GaruFolderBrowser *self)
{
  GtkTreeIter   iter;
  GSList       *folders, *files, *l;
  GtkListStore *liststore;
  GtkTreeView  *tree_view;
  gchar        *filename, *path_filename;

  tree_view = GTK_TREE_VIEW (self->tree_view);
  liststore = GTK_LIST_STORE (gtk_tree_view_get_model (tree_view));
  gtk_list_store_clear (liststore);

  /* gets first the folders and then music files */
  folders = garu_folder_browser_get_files (self->directory, FILE_TYPE_DIR);
  files = garu_folder_browser_get_files (self->directory, FILE_TYPE_MUSIC);

  /* displays the back button, this one is avaliable until the
     home directory */
  if (g_strcmp0 (self->directory, g_get_home_dir ()) != 0)
    {
      filename = g_path_get_basename (self->directory);
      path_filename = g_path_get_dirname (self->directory);
      gtk_list_store_append (liststore, &iter);
      gtk_list_store_set (liststore, &iter,
                          COLUMN_ICON, "go-previous-symbolic",
                          COLUMN_FILENAME, filename,
                          COLUMN_FILE_PATH, path_filename, -1);
      g_free (filename);
      g_free (path_filename);
    }

  /* add folders to GtkListStore*/
  for (l = folders; l != NULL; l = l->next)
    {
      filename = g_path_get_basename (l->data);
      gtk_list_store_append (liststore, &iter);
      gtk_list_store_set (liststore, &iter,
                          COLUMN_ICON, "folder-symbolic",
                          COLUMN_FILENAME, filename,
                          COLUMN_FILE_PATH, l->data,
                          COLUMN_TYPE, FILE_TYPE_DIR, -1);
      g_free (filename);
    }

  /* add files to GtkListStore */
  for (l = files; l != NULL; l = l->next)
    {
      filename = g_path_get_basename (l->data);
      gtk_list_store_append (liststore, &iter);
      gtk_list_store_set (liststore, &iter,
                          COLUMN_ICON, "emblem-music-symbolic",
                          COLUMN_FILENAME, filename,
                          COLUMN_FILE_PATH, l->data,
                          COLUMN_TYPE, FILE_TYPE_MUSIC, -1);
      g_free (filename);
    }

  g_slist_free_full (files, (GDestroyNotify) g_free);
  g_slist_free_full (folders, (GDestroyNotify) g_free);
}

/**
 * This function inspects the directory and extracts the folders or
 * music files found.
 */
static GSList *
garu_folder_browser_get_files (gchar *directory, gint type)
{
  GDir        *dir;
  gchar       *path_filename;
  GSList      *list = NULL;
  GError      *err = NULL;
  const gchar *filename = NULL;

  /* check if the any error in the directory */
  dir = g_dir_open (directory, 0, &err);
  if (err != NULL)
    {
      g_warning (err->message);
      return NULL;
    }

  /* read all the directory */
  while (filename = g_dir_read_name (dir))
    {
      path_filename = g_build_filename (directory, filename, NULL);
      if (garu_utils_file_is_hidden (path_filename))
        continue;
      switch (type)
        {
        case FILE_TYPE_DIR:
          if (g_file_test (path_filename, G_FILE_TEST_IS_DIR))
            list = g_slist_prepend (list, path_filename);
          break;
        case FILE_TYPE_MUSIC:
          if (g_file_test (path_filename, G_FILE_TEST_IS_REGULAR)
              && garu_utils_file_is_audio (path_filename))
            list = g_slist_prepend (list, path_filename);
          break;
        }
    }
  list = g_slist_sort (list, (GCompareFunc) g_strcmp0);
  g_dir_close (dir);
  return list;
}

static GSList *
garu_folder_browser_get_all_music_files (gchar *directory)
{
  GSList *subdirs, *subsongs, *l;

  subdirs = garu_folder_browser_get_files (directory, FILE_TYPE_DIR);
  subsongs = garu_folder_browser_get_files (directory, FILE_TYPE_MUSIC);

  for (l = subdirs; l != NULL; l = l->next)
    {
      GSList *subsubsongs;
      subsubsongs = garu_folder_browser_get_all_music_files (l->data);
      if (subsubsongs != NULL)
        subsongs = g_slist_concat (subsongs, subsubsongs);
    }

  g_slist_free_full (subdirs, (GDestroyNotify) g_free);
  return subsongs;
}

/* properties getters and setters */

static const gchar *
garu_folder_browser_get_directory (GaruFolderBrowser *self)
{
  return self->directory;
}

/**
 * Set the GaruFolderBrowser's directory. If directory is NULL then
 * use the home directory.
 */
static void
garu_folder_browser_set_directory (GaruFolderBrowser *self,
                                   const gchar       *directory)
{
  /* "default" is the default value in the settings gschema */
  if (g_strcmp0 (directory, "default") == 0 || directory == NULL)
    {
      self->directory = g_strdup (g_get_home_dir ());
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_DIRECTORY]);
      garu_folder_browser_load_directory (self);
    }
  else
    {
      if (g_strcmp0 (self->directory, directory) != 0)
        {
          g_free (self->directory);
          self->directory = g_strdup (directory);
          g_object_notify_by_pspec (G_OBJECT (self),
                                    properties [PROP_DIRECTORY]);
          garu_folder_browser_load_directory (self);
        }
    }
}

/* signals */

static void
garu_folder_browser_clicked_home_button (GtkButton         *button,
                                         GaruFolderBrowser *self)
{
  garu_folder_browser_set_directory (self, NULL);
}

static void
garu_folder_browser_clicked_browser_button (GtkButton         *button,
                                            GaruFolderBrowser *self)
{
  GtkWidget *dialog;
  GtkWindow *parent;
  gint       response;
  gchar     *directory;

  parent = garu_utils_get_active_window ();
  dialog = gtk_file_chooser_dialog_new (_("Open directory"), parent,
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        _("Open"), GTK_RESPONSE_OK,
                                        _("Cancel"), GTK_RESPONSE_CANCEL,
                                        NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_OK)
    {
      directory = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      garu_folder_browser_set_directory (self, directory);
      g_free (directory);
    }

  gtk_widget_destroy (dialog);
}

static gboolean
garu_folder_browser_button_press (GtkWidget         *widget,
                                  GdkEventButton    *event,
                                  GaruFolderBrowser *self)
{
  GtkTreeView      *tree_view;
  GtkTreeModel     *tree_model;
  GtkTreeSelection *selection;
  GtkTreeIter       iter;
  gchar            *file_path;
  gint              file_type;
  GList            *list, *l;

  if ((event->type == GDK_DOUBLE_BUTTON_PRESS)
      && (event->button == GDK_BUTTON_PRIMARY))
    {
      tree_view = GTK_TREE_VIEW (widget);
      tree_model = gtk_tree_view_get_model (tree_view);
      selection = gtk_tree_view_get_selection (tree_view);
      list = gtk_tree_selection_get_selected_rows (selection, &tree_model);
      for (l = list; l != NULL; l = l->next)
        {
          gtk_tree_model_get_iter (tree_model, &iter, l->data);
          gtk_tree_model_get (tree_model, &iter,
                              COLUMN_TYPE, &file_type,
                              COLUMN_FILE_PATH, &file_path, -1);
          if (file_type == FILE_TYPE_DIR)
            {
              garu_folder_browser_set_directory (self, file_path);
              /* Can only open a directory at once */
              break;
            }
          else
            {
              g_print ("Play %s\n", file_path);
              /* app = GARU_APPLICATION (g_application_get_default ()); */
              /* player = garu_application_get_player (app); */
              /* uri = g_filename_to_uri (file_path, NULL, &error); */
              /* garu_player_set_track (player, uri); */
              /* garu_player_play (player); */
            }
        }
      g_free (file_path);
      g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free);
    }

  return FALSE;
}

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
          gtk_tree_model_get_value (model, &iter, COLUMN_FILE_PATH, &dir);
          address = g_value_get_string (&dir);

          switch (g_value_get_int (&type))
            {
            case FILE_TYPE_DIR:
              addr = g_strdup (address);
              songs = garu_folder_browser_get_all_music_files (addr);
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
            case FILE_TYPE_MUSIC:
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
  return g_object_new (GARU_TYPE_FOLDER_BROWSER,
                       "orientation", GTK_ORIENTATION_VERTICAL,
                       NULL);
}
