/* garu-playbin.c
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

#include "garu-player.h"
#include "garu-utils.h"
#include "garu-playbin.h"
#include "garu-application.h"

struct _GaruPlaybin
{
  GObject parent;

  GstElement *playbin;
  GstElement *equalizer;
  GstElement *convert;
  GstElement *karaoke;
  GstElement *sink;

};

G_DEFINE_TYPE (GaruPlaybin, garu_playbin, G_TYPE_OBJECT);

static void     garu_playbin_init_playbin (GaruPlaybin *self);
static gboolean garu_playbin_bus_call     (GstBus        *bus,
					   GstMessage    *msg);

static void
garu_playbin_finalize (GObject *gobject)
{
  GaruPlaybin *self;
  self = GARU_PLAYBIN (gobject);
  g_print ("Finalize player bin\n");
  gst_element_set_state (self->playbin, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (self->playbin));
  G_OBJECT_CLASS (garu_playbin_parent_class)->finalize (gobject);
}

static void
garu_playbin_class_init (GaruPlaybinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = garu_playbin_finalize;
}

static void
garu_playbin_init (GaruPlaybin *self)
{
  garu_playbin_init_playbin (self);
}

static void
garu_playbin_init_playbin (GaruPlaybin *self)
{
  GstBus *bus;

  self->playbin = gst_element_factory_make ("playbin", "playbin");
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
      garu_playbin_update_equalizer (self);
    }
  else
    {
      self->equalizer = self->karaoke = self->convert = self->sink = NULL;
    }

  //g_object_set ( G_OBJECT(self->playbin), "volume", 1.0, NULL);
  bus = gst_pipeline_get_bus (GST_PIPELINE (self->playbin));
  gst_bus_add_watch (bus, (GstBusFunc) garu_playbin_bus_call, NULL);
  gst_object_unref (bus);

  /* also clean up */
  /* gst_element_set_state (self->playbin, GST_STATE_NULL); */
  /* gst_object_unref (GST_OBJECT (self->playbin)); */
}

static gboolean
garu_playbin_bus_call (GstBus *bus, GstMessage *msg)
{
  switch (GST_MESSAGE_TYPE (msg))
    {
    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      break;

    case GST_MESSAGE_ERROR:
      {
	GError          *error;
	GaruApplication *app;
	GaruPlayer      *player;
	app = GARU_APPLICATION (g_application_get_default ());
	player = garu_application_get_player (app);
	garu_player_stop (player);
	gst_message_parse_error (msg, &error, NULL);
	garu_application_show_message (app, error->message);
	g_error_free ( error);
      }
    }
  return TRUE;
}

GaruPlaybin *
garu_playbin_new (void)
{
  return g_object_new (GARU_TYPE_PLAYBIN, NULL);
}

void
garu_playbin_set_uri (GaruPlaybin *self, gchar *uri)
{
  g_object_set (G_OBJECT (self->playbin), "uri", uri, NULL);
}

void
garu_playbin_play (GaruPlaybin *self)
{
  gst_element_set_state (self->playbin, GST_STATE_PLAYING);
}

void
garu_playbin_pause (GaruPlaybin *self)
{
  gst_element_set_state (self->playbin, GST_STATE_PAUSED);
}

void
garu_playbin_stop (GaruPlaybin *self)
{
  gst_element_set_state (self->playbin, GST_STATE_NULL);
}

void
garu_playbin_set_volume (GaruPlaybin *self, gdouble value)
{
  g_object_set ( G_OBJECT(self->playbin), "volume", value, NULL);
}

void
garu_playbin_set_position (GaruPlaybin *self, gdouble fraction)
{
  gint64 position;
  position = GST_TIME_AS_NSECONDS (garu_playbin_get_duration(self));
  gst_element_seek (self->playbin, 1.0, GST_FORMAT_TIME,
                    GST_SEEK_FLAG_FLUSH,
                    GST_SEEK_TYPE_SET, position * fraction,
                    GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

gint64
garu_playbin_get_position (GaruPlaybin *self)
{
  gint64 position;
  gst_element_query_position (self->playbin, GST_FORMAT_TIME, &position);
  return position;
}

gint64
garu_playbin_get_duration (GaruPlaybin *self)
{
  gint64 duration;
  gst_element_query_duration (self->playbin, GST_FORMAT_TIME, &duration);
  return duration;
}

void
garu_playbin_equalizer_disabled (GaruPlaybin *self)
{
  g_object_set ( G_OBJECT(self->karaoke), "level", 0, NULL);
  
}

GstElement *
garu_playbin_get_equalizer (GaruPlaybin *self)
{
  return self->equalizer;
}

void
garu_playbin_disable_equalizer (GaruPlaybin *self)
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
garu_playbin_update_equalizer (GaruPlaybin *self)
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
      garu_playbin_disable_equalizer (self);
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
