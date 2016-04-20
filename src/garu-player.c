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
  gint           crossfade_times;
  gint           status;

  /* Plates */
  GaruPlayerBin *plate1;
  GaruPlayerBin *plate2;

  /* Tagger */
  GaruTagger    *tagger;

  /* Properties */
  gboolean       equalizer;
  gdouble        volume;
  gboolean       crossfade;
  gint           crossfade_seconds;
  gboolean       crossfade_change_track;
  gboolean       crossfade_track_ends;

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
  PROP_CROSSFADE,
  PROP_CROSSFADE_SECONDS,
  PROP_CROSSFADE_CHANGE_TRACK,
  PROP_CROSSFADE_TRACK_ENDS,
  LAST_PROP
};

static GParamSpec *properties [LAST_PROP];
static guint signals [LAST_SIGNAL];

static void garu_player_load_settings (GaruPlayer *self);
static void garu_player_do_crossfade (GaruPlayer *self);
static gboolean garu_player_crossfade (GaruPlayer *self);
static void garu_player_unref_plates (GaruPlayer *self);
static void garu_player_set_crossfade_enabled (GaruPlayer *self,
                                               gboolean   enabled);
static void garu_player_set_crossfade_seconds    (GaruPlayer *self,
						  gint        seconds);
static void garu_player_set_crossfade_change_track (GaruPlayer *self,
						    gboolean    enabled);
static void garu_player_set_crossfade_track_ends  (GaruPlayer *self,
						   gboolean    enabled);

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
    case PROP_CROSSFADE:
      garu_player_set_crossfade_enabled (self, g_value_get_boolean (value));
      break;
    case PROP_CROSSFADE_SECONDS:
      garu_player_set_crossfade_seconds (self, g_value_get_int (value));
      break;
    case PROP_CROSSFADE_CHANGE_TRACK:
      garu_player_set_crossfade_change_track (self,
					      g_value_get_boolean (value));
      break;
    case PROP_CROSSFADE_TRACK_ENDS:
      garu_player_set_crossfade_track_ends (self, g_value_get_boolean (value));
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
			 G_PARAM_WRITABLE);
  properties [PROP_EQUALIZER] =
    g_param_spec_boolean ("equalizer",
                          "Equalizer",
                          "The equalizer is enabled",
                          FALSE,
			  G_PARAM_WRITABLE);
  properties [PROP_CROSSFADE] =
    g_param_spec_boolean ("crossfade",
			  "Crossfade",
			  "Crossfade for the player",
			  TRUE,
			  G_PARAM_WRITABLE);
  properties [PROP_CROSSFADE_SECONDS] =
    g_param_spec_int ("crossfade-time",
		      "Crossfade time",
		      "Time for the transition",
		      0, 12, 3,
		      G_PARAM_WRITABLE);
  properties [PROP_CROSSFADE_CHANGE_TRACK] =
    g_param_spec_boolean ("crossfade-change-track",
                          "Crossfade change track",
                          "Crossfade when the user change the track",
                          TRUE,
			  G_PARAM_WRITABLE);
  properties [PROP_CROSSFADE_TRACK_ENDS] =
    g_param_spec_boolean ("crossfade-track-ends",
                          "Crossfade track ends",
                          "Crossfade before the track ends",
                          TRUE,
			  G_PARAM_WRITABLE);
  g_object_class_install_properties (object_class, LAST_PROP, properties);

  /* Signals */
  signals[PLAYING] =
    g_signal_new ("playing",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 0);
}

static void
garu_player_init (GaruPlayer *self)
{
  self->plate1 = garu_player_bin_new ();
  garu_player_bin_set_volume (self->plate1, self->volume);
  self->plate2 = NULL;
  self->crossfade = TRUE;
  self->crossfade_id = 0;
  self->crossfade_seconds = 2000;
  self->crossfade_times = 0;
  self->status = GARU_PLAYER_STATUS_STOPPED;
  self->tagger = garu_tagger_new ();

  garu_player_load_settings (self);
}

static void
garu_player_load_settings (GaruPlayer *self)
{
  GSettings       *settings;
  settings = garu_utils_get_settings ();

  g_settings_bind (settings, "volume",
		   self, "volume", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "equalizer-enabled",
		   self, "equalizer", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "crossfade-enabled",
		   self, "crossfade", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "crossfade-time",
		   self, "crossfade-time", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "crossfade-change-track",
		   self, "crossfade-change-track", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "crossfade-track-ends",
		   self, "crossfade-track-ends", G_SETTINGS_BIND_GET);
}

static void
garu_player_do_crossfade (GaruPlayer *self)
{
  gint interval;
  self->crossfade_times = 0;
  interval = self->crossfade_seconds * 1000 / GARU_TIMES; /* miliseconds */
  self->status = GARU_PLAYER_STATUS_CROSSFADING;
  garu_player_bin_set_volume (self->plate2, 0);
  garu_player_bin_play (self->plate2);
  self->crossfade_id =
    g_timeout_add (interval, (GSourceFunc) garu_player_crossfade, self);
}

static gboolean
garu_player_crossfade (GaruPlayer *self)
{
  gboolean response;
  gdouble  diff_volume, fraction;

  self->crossfade_times += 1;
  fraction = (self->volume / (gdouble) GARU_TIMES) * self->crossfade_times;
  if (self->crossfade_times < GARU_TIMES)
    {
      garu_player_bin_set_volume (self->plate1, self->volume-fraction);
      garu_player_bin_set_volume (self->plate2, fraction);
      response = TRUE;
    }
  else
    {
      garu_player_unref_plates (self);
      garu_player_bin_set_volume (self->plate1, self->volume);
      self->status = GARU_PLAYER_STATUS_PLAYING;
      response = FALSE;
    }
  return response;
}

static void
garu_player_unref_plates (GaruPlayer *self)
{
  g_object_unref (self->plate1);
  self->plate1 = g_object_ref (self->plate2);
  g_object_unref (self->plate2);
}

static void
garu_player_set_crossfade_enabled (GaruPlayer *self, gboolean enabled)
{
  self->crossfade = enabled;
}

static void
garu_player_set_crossfade_seconds (GaruPlayer *self, gint seconds)
{
  self->crossfade_seconds = seconds;
}

static void
garu_player_set_crossfade_change_track (GaruPlayer *self, gboolean enabled)
{
  self->crossfade_change_track = enabled;
}

static void
garu_player_set_crossfade_track_ends (GaruPlayer *self, gboolean enabled)
{
  self->crossfade_track_ends = enabled;
}

GaruPlayer *
garu_player_new (void)
{
  return g_object_new (GARU_TYPE_PLAYER, NULL);
}

void
garu_player_set_track (GaruPlayer *self, gchar *uri)
{
  gint position;
  switch (self->status)
    {
    case GARU_PLAYER_STATUS_STOPPED:
    case GARU_PLAYER_STATUS_PAUSED:
      garu_player_bin_set_uri (self->plate1, uri);
      break;
    case GARU_PLAYER_STATUS_PLAYING:
      position = GST_TIME_AS_MSECONDS (garu_player_get_position (self));
      if (self->crossfade && self->crossfade_change_track && position > 1000)
	{
	  self->plate2 = garu_player_bin_new ();
	  garu_player_bin_set_uri (self->plate2, uri);
	}
      else
	{
	  garu_player_stop (self);
	  garu_player_set_track (self, uri);
	}
      break;
    case GARU_PLAYER_STATUS_CROSSFADING:
      garu_player_stop (self);
      garu_player_bin_set_uri (self->plate1, uri);
      break;
    }
  garu_tagger_set_uri (self->tagger, uri);
}

void
garu_player_play (GaruPlayer *self)
{
  gint position;
  switch (self->status)
    {
    case GARU_PLAYER_STATUS_PLAYING:
      garu_player_do_crossfade (self);
      break;
    default:
      self->status = GARU_PLAYER_STATUS_PLAYING;
      garu_player_bin_play (self->plate1);
      break;
    }
  g_signal_emit (self, signals[PLAYING], 0);
}

void
garu_player_pause (GaruPlayer *self)
{
  switch (self->status)
    {
    case GARU_PLAYER_STATUS_CROSSFADING:
      garu_player_bin_set_volume (self->plate2, self->volume);
      garu_player_bin_pause (self->plate1);
      garu_player_bin_pause (self->plate2);
      g_source_remove (self->crossfade_id);
      garu_player_unref_plates (self);
      break;
    case GARU_PLAYER_STATUS_PLAYING:
      garu_player_bin_pause (self->plate1);
      break;
    }
  self->status = GARU_PLAYER_STATUS_PAUSED;
}

void
garu_player_stop (GaruPlayer *self)
{
  switch (self->status)
    {
    case GARU_PLAYER_STATUS_CROSSFADING:
      g_source_remove (self->crossfade_id);
      garu_player_bin_set_volume (self->plate2, self->volume);
      garu_player_bin_stop (self->plate1);
      garu_player_bin_stop (self->plate2);
      garu_player_unref_plates (self);
      break;
    case GARU_PLAYER_STATUS_PLAYING:
      garu_player_bin_stop (self->plate1);
      break;
    }
  self->status = GARU_PLAYER_STATUS_STOPPED;
}

gint
garu_player_get_status (GaruPlayer *self)
{
  return self->status;
}

void
garu_player_set_position (GaruPlayer *self, gdouble fraction)
{
  switch (self->status)
    {
    case GARU_PLAYER_STATUS_CROSSFADING:
      garu_player_bin_set_position (self->plate2, fraction);
      break;
    default:
      garu_player_bin_set_position (self->plate1, fraction);
      break;
    }
}

gint64
garu_player_get_position (GaruPlayer *self)
{
  gint64 position;
  switch (self->status)
    {
    case GARU_PLAYER_STATUS_PLAYING:
      position = garu_player_bin_get_position (self->plate1);
      break;
    case GARU_PLAYER_STATUS_CROSSFADING:
      position = garu_player_bin_get_position (self->plate2);
      break;
    default:
      position = 0;
      break;
    }
  return position;
}

gchar *
garu_player_get_position_str (GaruPlayer *self)
{
  gint64 position;
  position = GST_TIME_AS_SECONDS (garu_player_get_position (self));
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
    case GARU_PLAYER_STATUS_CROSSFADING:
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
  self->volume = volume;
  garu_player_bin_set_volume (self->plate1, volume);
}

gdouble
garu_player_get_volume (GaruPlayer *self)
{
  return self->volume;
}
