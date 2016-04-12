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
#include "garu-player-bin.h"

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
garu_player_bin_dispose (GObject *gobject)
{
  GaruPlayerBin *self;
  self = GARU_PLAYER_BIN (gobject);
  gst_element_set_state (self->playbin, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (self->playbin));
  G_OBJECT_CLASS (garu_player_bin_parent_class)->dispose (gobject);
}

static void
garu_player_bin_init (GaruPlayerBin *self)
{
  garu_player_bin_init_playbin (self);
}

static void
garu_player_bin_class_init (GaruPlayerBinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = garu_player_bin_dispose;
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
    }
  else
    self->equalizer = self->karaoke = self->convert = self->sink = NULL;

  // g_object_set ( G_OBJECT(self->playbin), "volume", 5.0, NULL);
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
