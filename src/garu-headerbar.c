/* garu-headerbar.c
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

#include <gst/gst.h>

#include "garu-application.h"

#include "garu-headerbar.h"
#include "garu-utils.h"
#include "garu-song-box.h"

struct _GaruHeaderbar
{
  GtkHeaderBar parent;

  /* buttons */
  GtkWidget *prev_button;
  GtkWidget *play_button;
  GtkWidget *stop_button;
  GtkWidget *next_button;
  GtkWidget *settings_button;

  /* song information */
  GaruSongBox *song_box;
  guint        timeout_id;
};

G_DEFINE_TYPE (GaruHeaderbar, garu_headerbar, GTK_TYPE_HEADER_BAR);

enum {
  PROP_0,
  PROP_SHOW_SETTINGS_BUTTON,
  LAST_PROP
};

static GParamSpec *properties [LAST_PROP];

static void       garu_headerbar_init_song_box            (GaruHeaderbar  *self);
static void       garu_headerbar_init_control_buttons     (GaruHeaderbar  *self);
static void       garu_headerbar_init_playback_buttons    (GaruHeaderbar  *self);
static GtkWidget *garu_headerbar_new_button               (const gchar    *name,
                                                           gboolean        toggle);
static void       garu_headerbar_set_button_image         (GtkButton      *button,
                                                           const gchar    *name);
static gboolean   garu_headerbar_sync_progress            (GaruHeaderbar  *self);
static void       garu_headerbar_sync_progress_stop       (GaruHeaderbar  *self);

/* Signals */
static void       garu_headerbar_play_button_clicked      (GtkButton      *button,
                                                           GaruHeaderbar  *self);
static void       garu_headerbar_stop_button_clicked      (GtkButton      *button,
                                                           GaruHeaderbar  *self);
static void       garu_headerbar_next_button_clicked      (GtkButton      *button,
                                                           GaruHeaderbar  *self);
static void       garu_headerbar_prev_button_clicked      (GtkButton      *button,
                                                           GaruHeaderbar  *self);
static void       garu_headerbar_volume_change            (GtkScaleButton *button,
                                                           gdouble         value);
static void       garu_headerbar_new_playing              (GaruPlayer     *player,
                                                           GaruHeaderbar  *self);
static void       garu_headerbar_set_show_settings_button (GaruHeaderbar  *self,
                                                           gboolean value);
static GtkWidget *garu_headerbar_create_settings_button   (GaruHeaderbar *self);

static void
garu_headerbar_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  // GaruApplication *self = GARU_APPLICATION (object);
}

static void
garu_headerbar_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  GaruHeaderbar *self = GARU_HEADERBAR (object);
  switch (property_id)
    {
    case PROP_SHOW_SETTINGS_BUTTON:
      garu_headerbar_set_show_settings_button (self,
                                               g_value_get_boolean (value));
      break;
    }
}

static void
garu_headerbar_class_init (GaruHeaderbarClass *klass)
{
  GObjectClass      *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = garu_headerbar_get_property;
  object_class->set_property = garu_headerbar_set_property;

  /* properties */
  properties [PROP_SHOW_SETTINGS_BUTTON] =
    g_param_spec_boolean ("show-settings-button",
                          "Show settings button",
                          "Show the settings button in the GtkHeaderBar",
                          FALSE, G_PARAM_WRITABLE);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
garu_headerbar_init (GaruHeaderbar *self)
{
  GaruApplication *app;
  GSettings       *settings;
  GaruPlayer      *player;

  self->timeout_id = 0;

  garu_headerbar_init_playback_buttons (self);
  garu_headerbar_init_song_box (self);
  garu_headerbar_init_control_buttons (self);

  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (self), TRUE);

  app = GARU_APPLICATION (g_application_get_default ());
  settings = garu_application_get_settings (app);
  player = garu_application_get_player (app);


  /* signals */
  g_settings_bind (settings, "show-settings-button",
                   self, "show-settings-button", G_SETTINGS_BIND_GET);
  g_signal_connect (player, "playing",
                    G_CALLBACK (garu_headerbar_new_playing), self);
}

static void
garu_headerbar_init_song_box (GaruHeaderbar *self)
{
  GtkWidget *song_box;
  song_box = garu_song_box_new ();
  gtk_header_bar_set_custom_title(GTK_HEADER_BAR(self), song_box);
  gtk_widget_show (song_box);
  self->song_box = GARU_SONG_BOX (song_box);
}

static void
garu_headerbar_init_control_buttons (GaruHeaderbar *self)
{
  GtkWidget *box, *prev_button, *play_button, *stop_button, *next_button;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  /* previous */
  prev_button =
    garu_headerbar_new_button ("media-skip-backward-symbolic", FALSE);
  gtk_button_set_relief (GTK_BUTTON(prev_button), GTK_RELIEF_NONE);
  g_signal_connect (prev_button, "clicked",
                    G_CALLBACK (garu_headerbar_prev_button_clicked), self);
  gtk_box_pack_start (GTK_BOX (box), prev_button, FALSE, FALSE, 0);
  /* play */
  play_button =
    garu_headerbar_new_button ("media-playback-start-symbolic", FALSE);
  gtk_button_set_relief (GTK_BUTTON(play_button), GTK_RELIEF_NONE);
  g_signal_connect (play_button, "clicked",
                    G_CALLBACK (garu_headerbar_play_button_clicked), self);
  gtk_box_pack_start (GTK_BOX (box), play_button, FALSE, FALSE, 0);
  /* stop */
  stop_button =
    garu_headerbar_new_button ("media-playback-stop-symbolic", FALSE);
  gtk_button_set_relief (GTK_BUTTON(stop_button), GTK_RELIEF_NONE);
  gtk_widget_set_sensitive (stop_button, FALSE);
  g_signal_connect (stop_button, "clicked",
                    G_CALLBACK (garu_headerbar_stop_button_clicked), self);
  gtk_box_pack_start (GTK_BOX (box), stop_button, FALSE, FALSE, 0);
  /* next */
  next_button =
    garu_headerbar_new_button ("media-skip-forward-symbolic", FALSE);
  gtk_button_set_relief (GTK_BUTTON(next_button), GTK_RELIEF_NONE);
  g_signal_connect (next_button, "clicked",
                    G_CALLBACK (garu_headerbar_next_button_clicked), self);
  gtk_box_pack_start (GTK_BOX (box), next_button, FALSE, FALSE, 0);

  gtk_header_bar_pack_start (GTK_HEADER_BAR (self), box);
  gtk_widget_show_all (box);

  self->prev_button = prev_button;
  self->play_button = play_button;
  self->stop_button = stop_button;
  self->next_button = next_button;
}

static void
garu_headerbar_init_playback_buttons (GaruHeaderbar *self)
{
  GSettings       *settings;
  GaruApplication *app;
  GtkWidget       *box, *button, *image;

  app = GARU_APPLICATION (g_application_get_default ());
  settings = garu_application_get_settings (app);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);

  /* shuffle */
  button = garu_headerbar_new_button ("media-playlist-shuffle-symbolic", TRUE);
  gtk_button_set_relief (GTK_BUTTON(button), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  gtk_widget_show (button);
  /* repeat */
  button = garu_headerbar_new_button ("media-playlist-repeat-symbolic", TRUE);
  gtk_button_set_relief (GTK_BUTTON(button), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  gtk_widget_show (button);
  /* volume */
  button = gtk_volume_button_new ();
  gtk_scale_button_set_value (GTK_SCALE_BUTTON (button), 1);
  g_settings_bind (settings, "volume", button, "value",
                   G_SETTINGS_BIND_DEFAULT);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  gtk_header_bar_pack_end (GTK_HEADER_BAR (self), box);
  gtk_widget_show (box);
}

static GtkWidget *
garu_headerbar_new_button (const gchar *name, gboolean toggle)
{
  GtkWidget *button, *image;
  if (toggle)
    button = gtk_toggle_button_new ();
  else
    button = gtk_button_new ();
  image = gtk_image_new_from_icon_name (name, GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_button_set_image (GTK_BUTTON (button), image);
  return button;
}

static void
garu_headerbar_set_button_image (GtkButton *button, const gchar *name)
{
  GtkWidget *image;
  image = gtk_button_get_image (button);
  gtk_image_set_from_icon_name (GTK_IMAGE (image), name,
                                GTK_ICON_SIZE_SMALL_TOOLBAR);
}

static gboolean
garu_headerbar_sync_progress (GaruHeaderbar *self)
{
  GaruApplication *app;
  GaruPlayer      *player;
  GaruTagger      *tagger;
  gint             pos, len;
  gdouble          fraction;

  app = GARU_APPLICATION (g_application_get_default ());
  player = garu_application_get_player (app);

  if (garu_player_get_status (player) == GARU_PLAYER_STATUS_STOPPED)
    {
      garu_headerbar_sync_progress_stop (self);
      return FALSE;
    }

  tagger = garu_player_get_tagger (player);
  len = garu_tagger_get_length (tagger);
  pos = GST_TIME_AS_SECONDS (garu_player_get_position (player));
  fraction =  pos / (gdouble) len;
  garu_song_box_progress_bar_set_fraction (self->song_box, fraction);
  garu_song_box_set_position (self->song_box, pos);

  if (fraction >= 1)
    return FALSE;
  return TRUE;
}

static void
garu_headerbar_sync_progress_stop (GaruHeaderbar *self)
{
  if (self->timeout_id != 0)
    {
      g_source_remove (self->timeout_id);
      self->timeout_id = 0;
    }
}

/* Signals */

static void
garu_headerbar_play_button_clicked (GtkButton *button, GaruHeaderbar *self)
{
  GaruApplication *app;
  GaruPlaylist    *playlist;
  GaruPlayer      *player;
  gchar           *file;

  app = GARU_APPLICATION (g_application_get_default ());
  player = garu_application_get_player (app);
  playlist = garu_application_get_playlist (app);

  switch (garu_player_get_status (player))
    {
    case GARU_PLAYER_STATUS_STOPPED:
      file = garu_playlist_get_track (playlist, GARU_PLAYLIST_GET_START_TRACK);
      if (file == NULL)
        return;
      garu_player_set_track (player, g_filename_to_uri (file, NULL, NULL));
      gtk_widget_set_sensitive (self->stop_button, TRUE);
      g_free (file);
    case GARU_PLAYER_STATUS_PAUSED:
      garu_player_play (player);
      break;
    case GARU_PLAYER_STATUS_PLAYING:
    case GARU_PLAYER_STATUS_CROSSFADING:
      garu_player_pause (player);
      garu_headerbar_set_button_image (button, "media-playback-start-symbolic");
      garu_headerbar_sync_progress_stop (self);
      break;
    }
}

static void
garu_headerbar_stop_button_clicked (GtkButton *button, GaruHeaderbar *self)
{
  GaruApplication *app;
  GaruPlayer      *player;
  GaruPlaylist    *playlist;

  app = GARU_APPLICATION (g_application_get_default ());
  player = garu_application_get_player (app);
  playlist = garu_application_get_playlist (app);
  garu_headerbar_set_button_image (GTK_BUTTON (self->play_button),
                                   "media-playback-start-symbolic");
  garu_headerbar_sync_progress_stop (self);
  gtk_widget_set_sensitive (self->stop_button, FALSE);
  garu_song_box_progress_bar_set_fraction (self->song_box, 0);
  garu_song_box_default_title (self->song_box);
  garu_playlist_free (playlist);
  garu_player_stop (player);
}

static void
garu_headerbar_next_button_clicked (GtkButton *button, GaruHeaderbar *self)
{
  GaruApplication *app;
  GaruPlayer      *player;
  GaruPlaylist    *playlist;
  gchar           *file;

  app = GARU_APPLICATION (g_application_get_default ());
  player = garu_application_get_player (app);
  playlist = garu_application_get_playlist (app);

  file = garu_playlist_get_track (playlist, GARU_PLAYLIST_GET_NEXT);

  if (file == NULL)
    return;

  garu_player_set_track (player, g_filename_to_uri (file, NULL, NULL));
  garu_player_play (player);

  g_free (file);
}

static void
garu_headerbar_prev_button_clicked (GtkButton *button, GaruHeaderbar *self)
{
  GaruApplication *app;
  GaruPlayer      *player;
  GaruPlaylist    *playlist;
  gchar           *file;

  app = GARU_APPLICATION (g_application_get_default ());
  player = garu_application_get_player (app);
  playlist = garu_application_get_playlist (app);

  file = garu_playlist_get_track (playlist, GARU_PLAYLIST_GET_PREV);

  if (file == NULL)
    return;

  garu_player_set_track (player, g_filename_to_uri (file, NULL, NULL));
  garu_player_play (player);

  g_free (file);
}

static void
garu_headerbar_new_playing (GaruPlayer *player, GaruHeaderbar *self)
{
  GaruTagger  *tagger;
  gint         length;
  gchar       *title, *artist;

  tagger = garu_player_get_tagger (player);
  title = garu_tagger_get_title (tagger);
  artist = garu_tagger_get_artist (tagger);

  garu_headerbar_sync_progress_stop (self);
  garu_headerbar_set_button_image (GTK_BUTTON (self->play_button),
                                   "media-playback-pause-symbolic");
  length = garu_tagger_get_length (tagger);
  garu_song_box_set_length (self->song_box, length);
  garu_song_box_set_title (self->song_box, title, artist);

  if (garu_song_box_progress_bar_get_fraction (self->song_box) == 0)
    garu_song_box_set_position (self->song_box, 0);

  self->timeout_id =
    g_timeout_add (500, (GSourceFunc) garu_headerbar_sync_progress, self);
  gtk_widget_show_all (GTK_WIDGET (self->song_box));
  g_free (artist);
  g_free (title);
}

static void
garu_headerbar_set_show_settings_button (GaruHeaderbar *self, gboolean value)
{
  GaruApplication *app;

  app = GARU_APPLICATION (g_application_get_default ());
  if (!gtk_application_prefers_app_menu (GTK_APPLICATION (app)) || value)
    {
      self->settings_button = garu_headerbar_create_settings_button (self);
    }
  else
    {
      if (self->settings_button)
        gtk_widget_destroy (self->settings_button);
    }
}

static GtkWidget *
garu_headerbar_create_settings_button (GaruHeaderbar *self)
{
  GList           *list;
  GtkWidget       *button, *image;
  GaruApplication *app;
  GMenuModel      *menu_model;

  app = GARU_APPLICATION (g_application_get_default ());

  list = gtk_container_get_children (GTK_CONTAINER (self));
  list = g_list_last (list);
  button = gtk_menu_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_icon_name ("emblem-system-symbolic",
                                        GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (list->data), button, FALSE, FALSE, 0);

  /* create menu */
  menu_model = G_MENU_MODEL (garu_application_get_menu (app));
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (button), menu_model);
  g_print ("Removiendo\n");
  g_object_unref (menu_model);

  return button;
}

GtkWidget *
garu_headerbar_new (void)
{
  return g_object_new (GARU_TYPE_HEADERBAR, NULL);
}
