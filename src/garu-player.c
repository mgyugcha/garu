/* garu-player.c
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
#include "garu-player.h"
#include "garu-player-bin.h"
#include "garu-utils.h"

#define GARU_TIMES 20

struct _GaruPlayer
{
  GObject parent;

  /* Crossfade */
  guint          crossfade_id;
  gint           crossfade_time;
  gint           crossfade_n_times;
  gdouble        volume_crossfade;
  gboolean       crossfade;
  gint           status;

  /* Plates */
  GaruPlayerBin *plate1;
  GaruPlayerBin *plate2;

  /* Tagger */
  GaruTagger    *tagger;

  /* Properties */
  gboolean       equalizer;
  gdouble        volume;

};

G_DEFINE_TYPE (GaruPlayer, garu_player, G_TYPE_OBJECT);

enum {
  PLAYING,
  PAUSED,
  STOPPED,
  CROSSFADING,
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_VOLUME,
  PROP_EQUALIZER,
  LAST_PROP
};

static GParamSpec *properties [LAST_PROP];

static guint signals[LAST_SIGNAL];

static void garu_player_load_settings (GaruPlayer *self);
static void garu_player_do_crossfade (GaruPlayer *self);
static gboolean garu_player_crossfade (GaruPlayer *self);

static void
garu_player_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  GaruPlayer *self = GARU_PLAYER (object);

  switch (property_id)
    {
    case PROP_VOLUME:
      g_value_set_double (value, garu_player_get_volume (self));
      break;
    case PROP_EQUALIZER:
      g_value_set_boolean (value, garu_player_get_equalizer_enabled (self));
      break;
    }
}

static void
garu_player_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  GaruPlayer *self = GARU_PLAYER (object);

  switch (property_id)
    {
    case PROP_VOLUME:
      garu_player_set_volume (self, g_value_get_double (value));
      break;
    case PROP_EQUALIZER:
      garu_player_set_equalizer_enabled (self, g_value_get_boolean (value));
      break;
    }
}

static void
garu_player_class_init (GaruPlayerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = garu_player_get_property;
  object_class->set_property = garu_player_set_property;

  /* Properties */
  properties [PROP_VOLUME] =
    g_param_spec_double ("volume",
			 "Volume",
			 "Volume for the player",
			 0.0, 1.0, 1.0,
			 G_PARAM_READWRITE);
  properties [PROP_EQUALIZER] =
    g_param_spec_boolean ("equalizer",
                          "Equalizer",
                          "The equalizer is enabled",
                          FALSE,
                          G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  /* Signals */
  signals[PLAYING] =
    g_signal_new ("playing",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 0);

  /* signals[PAUSED] = */
  /*   g_signal_new ("paused", */
  /*                 G_TYPE_FROM_CLASS (klass), */
  /*                 G_SIGNAL_RUN_FIRST, */
  /*                 0, */
  /*                 NULL, NULL, NULL, */
  /*                 G_TYPE_NONE, 0); */
}

static void
garu_player_init (GaruPlayer *self)
{
  self->plate1 = garu_player_bin_new ();
  garu_player_bin_set_volume (self->plate1, self->volume);
  self->plate2 = NULL;
  self->crossfade = TRUE;
  self->crossfade_id = 0;
  self->crossfade_time = 2000;
  self->crossfade_n_times = 0;
  self->status = GARU_PLAYER_STOPPED;
  self->tagger = garu_tagger_new ();

  garu_player_load_settings (self);
}

static void
garu_player_load_settings (GaruPlayer *self)
{
  GSettings       *settings;
  GaruApplication *app;

  app = GARU_APPLICATION (g_application_get_default ());
  settings = garu_application_get_settings (app);

  g_settings_bind (settings, "equalizer-enabled", self, "equalizer",
		   G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "volume", self, "volume",
		   G_SETTINGS_BIND_GET);
}

static void
garu_player_do_crossfade (GaruPlayer *self)
{
  gint interval;
  self->crossfade_n_times = 0;
  self->crossfade_id = 0;

  interval = self->crossfade_time / GARU_TIMES; /* miliseconds */
  self->status = GARU_PLAYER_CROSSFADING;
  garu_player_bin_set_volume (self->plate2, 0);
  garu_player_bin_play (self->plate2);
  self->crossfade_id =
    g_timeout_add (interval, (GSourceFunc) garu_player_crossfade, self);
}

static gboolean
garu_player_crossfade (GaruPlayer *self)
{
  gboolean response;
  gdouble diff_volume, fraction;

  self->crossfade_n_times += 1;
  fraction = (self->volume/(gdouble)GARU_TIMES)* self->crossfade_n_times;

  if (self->crossfade_n_times <= GARU_TIMES)
    {
      garu_player_bin_set_volume (self->plate1, self->volume-fraction);
      garu_player_bin_set_volume (self->plate2, fraction);
      response = TRUE;
    }
  else
    {
      /* Free memory */
      g_object_unref (self->plate1);
      self->plate1 = self->plate2;
      self->plate2 = NULL;

      self->status = GARU_PLAYER_PLAYING;
      response = FALSE;
    }

  return response;
}

GaruPlayer *
garu_player_new (void)
{
  return g_object_new (GARU_TYPE_PLAYER, NULL);
}

void
garu_player_set_track (GaruPlayer *self, gchar *uri)
{
  /* Tags for the song */
  garu_tagger_set_uri (self->tagger, uri);

  switch (self->status)
    {
    case GARU_PLAYER_STOPPED:
    case GARU_PLAYER_PAUSED:
      garu_player_bin_set_uri (self->plate1, uri);
      break;
    case GARU_PLAYER_PLAYING:
      self->plate2 = garu_player_bin_new ();
      garu_player_bin_set_uri (self->plate2, uri);
      break;
    case GARU_PLAYER_CROSSFADING:
      g_print ("crosfunfading\n");
      /* free memory */
      g_source_remove (self->crossfade_id);
      g_object_unref (self->plate2);
      garu_player_stop (self);
      garu_player_set_volume (self, self->volume);
      garu_player_set_track (self, uri);
      break;
    }
}

void
garu_player_play (GaruPlayer *self)
{
  switch (self->status)
    {
    case GARU_PLAYER_PLAYING:
      garu_player_do_crossfade (self);
      break;
    default:
      self->status = GARU_PLAYER_PLAYING;
      garu_player_bin_play (self->plate1);
      break;
    }
  g_signal_emit (self, signals[PLAYING], 0);
}

void
garu_player_pause (GaruPlayer *self)
{
  if (self->status == GARU_PLAYER_CROSSFADING)
    {
      g_print ("player cross\n");
      garu_player_bin_pause (self->plate1);
      garu_player_bin_pause (self->plate2);

      /* Free memory */
      g_source_remove (self->crossfade_id);
      g_object_unref (self->plate1);
      self->plate1 = self->plate2;
      self->plate2 = NULL;
      garu_player_bin_set_volume (self->plate1, self->volume);
    }
  else
    garu_player_bin_pause (self->plate1);
  self->status = GARU_PLAYER_PAUSED;
}

void
garu_player_stop (GaruPlayer *self)
{
  switch (self->status)
    {
    case GARU_PLAYER_CROSSFADING:
      garu_player_bin_stop (self->plate1);
      garu_player_bin_stop (self->plate2);
      g_source_remove (self->crossfade_id);
      g_object_unref (self->plate1);
      self->plate1 = self->plate2;
      self->plate2 = NULL;
      garu_player_bin_set_volume (self->plate1, self->volume);
      break;
    case GARU_PLAYER_PLAYING:
      garu_player_bin_stop (self->plate1);
      break;
    }
  self->status = GARU_PLAYER_STOPPED;
}

gint
garu_player_get_status (GaruPlayer *self)
{
  return self->status;
}

gint64
garu_player_get_position (GaruPlayer *self)
{
  gint64 position;
  switch (self->status)
    {
    case GARU_PLAYER_PLAYING:
      position = garu_player_bin_get_position (self->plate1);
      break;
    case GARU_PLAYER_CROSSFADING:
      position = garu_player_bin_get_position (self->plate2);
      break;
    default:
      position = 0;
      break;
    }
  return GST_TIME_AS_SECONDS (position);
}

gchar *
garu_player_get_position_str (GaruPlayer *self)
{
  gint64 position;
  position = garu_player_get_position (self);
  return garu_utils_convert_seconds (position);
}

GaruTagger *
garu_player_get_tagger (GaruPlayer *self)
{
  return self->tagger;
}

void
garu_player_update_equalizer (GaruPlayer *self)
{
  switch (self->status)
    {
    case GARU_PLAYER_CROSSFADING:
      garu_player_bin_update_equalizer (self->plate2);
    default:
      garu_player_bin_update_equalizer (self->plate1);
      break;
    }
}

void
garu_player_set_equalizer_enabled (GaruPlayer *self, gboolean enabled)
{
  self->equalizer = enabled;
  garu_player_update_equalizer (self);
}

gboolean
garu_player_get_equalizer_enabled (GaruPlayer *self)
{
  return self->equalizer;
}

void
garu_player_set_volume (GaruPlayer *self, gdouble volume)
{
  self->volume_crossfade = self->volume = volume;
  garu_player_bin_set_volume (self->plate1, volume);
}

gdouble
garu_player_get_volume (GaruPlayer *self)
{
  return self->volume;
}
