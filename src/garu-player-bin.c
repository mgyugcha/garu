/* garu-player-bin.c
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

#include "garu-utils.h"
#include "garu-player-bin.h"
#include "garu-application.h"

struct _GaruPlayerBin
{
  GObject parent;

  GstElement *playbin;
  GstElement *equalizer;
  GstElement *convert;
  GstElement *karaoke;
  GstElement *sink;

};

G_DEFINE_TYPE (GaruPlayerBin, garu_player_bin, G_TYPE_OBJECT);

static void     garu_player_bin_init_playbin (GaruPlayerBin *self);
static gboolean bus_call                     (GstBus *bus,
                                              GstMessage *msg,
                                              gpointer data);

static void
garu_player_bin_finalize (GObject *gobject)
{
  GaruPlayerBin *self;
  self = GARU_PLAYER_BIN (gobject);
  g_print ("Finalize player bin\n");
  gst_element_set_state (self->playbin, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (self->playbin));
  G_OBJECT_CLASS (garu_player_bin_parent_class)->finalize (gobject);
}

static void
garu_player_bin_class_init (GaruPlayerBinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = garu_player_bin_finalize;
}

static void
garu_player_bin_init (GaruPlayerBin *self)
{
  garu_player_bin_init_playbin (self);
}

static void
garu_player_bin_init_playbin (GaruPlayerBin *self)
{
  GstBus *bus;

  self->playbin = gst_element_factory_make ("playbin", "play");
  self->equalizer = gst_element_factory_make ("equalizer-10bands", "equalizer");
  self->convert = gst_element_factory_make ("audioconvert", "convert");
  self->karaoke = gst_element_factory_make ("audiokaraoke", "karaoke");
  self->sink = gst_element_factory_make ("autoaudiosink", "audio-sink");

  if (self->playbin == NULL
      || self->convert == NULL
      || self->equalizer == NULL
      || self->karaoke == NULL
      || self->sink == NULL) 
    g_warning ("Install Gstreamer Base Plugins");

  if (self->equalizer != NULL && self->karaoke != NULL
      && self->convert != NULL && self->sink != NULL)
    {
      GstElement *bin;
      GstPad *pad, *ghost_pad;

      bin = gst_bin_new ("audiobin");
      gst_bin_add_many (GST_BIN(bin), self->equalizer, self->karaoke,
                        self->convert, self->sink, NULL);
      gst_element_link_many (self->equalizer, self->karaoke,
                             self->convert, self->sink, NULL);

      pad = gst_element_get_static_pad (self->equalizer, "sink");
      ghost_pad = gst_ghost_pad_new ("sink", pad);
      gst_pad_set_active (ghost_pad, TRUE);
      gst_element_add_pad (bin, ghost_pad);
      gst_object_unref (pad);

      g_object_set (self->playbin, "audio-sink", bin, NULL);

      g_object_set ( G_OBJECT(self->karaoke), "level", 0, NULL);
      garu_player_bin_update_equalizer (self);
    }
  else
    {
      self->equalizer = self->karaoke = self->convert = self->sink = NULL;
    }

  //g_object_set ( G_OBJECT(self->playbin), "volume", 1.0, NULL);
  bus = gst_pipeline_get_bus (GST_PIPELINE (self->playbin));
  gst_bus_add_watch (bus, bus_call, NULL);
  gst_object_unref (bus);

  /* also clean up */
  /* gst_element_set_state (self->playbin, GST_STATE_NULL); */
  /* gst_object_unref (GST_OBJECT (self->playbin)); */
}

static gboolean
bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  switch (GST_MESSAGE_TYPE (msg)) {

  case GST_MESSAGE_EOS:
    g_print ("End of stream\n");
    break;

  case GST_MESSAGE_ERROR: {
    gchar  *debug;
    GError *error;

    gst_message_parse_error (msg, &error, &debug);
    g_free (debug);

    g_printerr ("Error: %s\n", error->message);
    g_error_free (error);
    break;
  }
  default:
    break;
  }
  return TRUE;
}

GaruPlayerBin *
garu_player_bin_new (void)
{
  return g_object_new (GARU_TYPE_PLAYER_BIN, NULL);
}

void
garu_player_bin_set_uri (GaruPlayerBin *self, gchar *uri)
{
  g_object_set (G_OBJECT (self->playbin), "uri", uri, NULL);
}

void
garu_player_bin_play (GaruPlayerBin *self)
{
  gst_element_set_state (self->playbin, GST_STATE_PLAYING);
}

void
garu_player_bin_pause (GaruPlayerBin *self)
{
  gst_element_set_state (self->playbin, GST_STATE_PAUSED);
}

void
garu_player_bin_stop (GaruPlayerBin *self)
{
  gst_element_set_state (self->playbin, GST_STATE_NULL);
}

void
garu_player_bin_set_volume (GaruPlayerBin *self, gdouble value)
{
  g_object_set ( G_OBJECT(self->playbin), "volume", value, NULL);
}

gint64
garu_player_bin_get_position (GaruPlayerBin *self)
{
  gint64 position;
  gst_element_query_position (self->playbin, GST_FORMAT_TIME, &position);
  return position;
}

gint64
garu_player_bin_get_duration (GaruPlayerBin *self)
{
  gint64 duration;
  gst_element_query_duration (self->playbin, GST_FORMAT_TIME, &duration);
  return duration;
}

void
garu_player_bin_equalizer_disabled (GaruPlayerBin *self)
{
  g_object_set ( G_OBJECT(self->karaoke), "level", 0, NULL);
  
}

GstElement *
garu_player_bin_get_equalizer (GaruPlayerBin *self)
{
  return self->equalizer;
}

void
garu_player_bin_disable_equalizer (GaruPlayerBin *self)
{
  gchar *band;
  gint   i;

  for (i = 0; i < G_N_ELEMENTS (eq_bands); i++)
    {
      band = g_strdup_printf ("band%d", i);
      g_object_set (self->equalizer, band, 0.0, NULL);
      g_free (band);
    }
}

void
garu_player_bin_update_equalizer (GaruPlayerBin *self)
{
  GaruApplication *app;
  GSettings       *settings;
  gint             preset, i;
  gchar           *band, *settings_band;

  app = GARU_APPLICATION (g_application_get_default ());
  settings = garu_application_get_settings (app);
  preset = g_settings_get_int (settings, "equalizer-preset");

  if (!g_settings_get_boolean (settings, "equalizer-enabled"))
    {
      garu_player_bin_disable_equalizer (self);
      return;
    }

  for (i = 0; i < G_N_ELEMENTS (eq_bands); i++)
    {
      band = g_strdup_printf ("band%d", i);
      if (preset == G_N_ELEMENTS (eq_presets) - 1)
        {
          settings_band = g_strdup_printf ("eq-custom-band%d", i);
          g_object_set (self->equalizer, band,
                        g_settings_get_double (settings, settings_band), NULL);
          g_free (settings_band);
        }
      else
        {
          g_object_set (self->equalizer, band,
                        eq_presets_values[preset][i], NULL);
        }
      g_free (band);
    }
}
