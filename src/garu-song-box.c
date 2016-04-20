/* garu-song-box.c
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

#include "garu-song-box.h"
#include "garu-utils.h"

struct _GaruSongBox
{
  GtkBox     parent;

  GtkWidget *position;
  GtkWidget *progress_box;
  GtkWidget *progress_bar;
  GtkWidget *length;
  GtkWidget *label_title;
  GtkWidget *album_art;
};

G_DEFINE_TYPE (GaruSongBox, garu_song_box, GTK_TYPE_BOX);

static void       garu_song_box_init_elements              (GaruSongBox    *self);
static GtkWidget *garu_song_box_init_progress_box_elements (GaruSongBox    *self);
static void       garu_song_box_progress_bar_event_seek    (GtkWidget      *widget,
                                                            GdkEventButton *event,
                                                            GaruSongBox    *self);

static void
garu_song_box_get_preferred_width (GtkWidget *widget,
                                   gint      *minimum,
                                   gint      *natural)
{
  if (minimum != NULL)
    *minimum = 250;
  if (natural != NULL)
    *natural = 1600;
}

static void
garu_song_box_class_init (GaruSongBoxClass *klass)
{
  GtkWidgetClass *class;
  class = GTK_WIDGET_CLASS (klass);
  class->get_preferred_width = garu_song_box_get_preferred_width;
}

static void
garu_song_box_init (GaruSongBox *self)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self),
                                  GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_set_margin_start (GTK_WIDGET (self), 10);
  gtk_widget_set_margin_end (GTK_WIDGET (self), 10);
  garu_song_box_init_elements (self);
}

static void
garu_song_box_init_elements (GaruSongBox *self)
{
  //GdkPixbuf *pixbuf;
  GtkWidget *hbox, *vbox, *label;
  //, *album_art;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  /* Title: song title - artist */
  label = gtk_label_new (NULL);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  /* Progress box: 02:33 [==============-------] 03:39 */
  garu_song_box_init_progress_box_elements (self);
  gtk_box_pack_start (GTK_BOX (vbox), self->progress_box, FALSE, FALSE, 0);

  /*
  pixbuf = gdk_pixbuf_new_from_file ("album.png", NULL);
  pixbuf = gdk_pixbuf_scale_simple (pixbuf, 34, 34, GDK_INTERP_BILINEAR);
  album_art = gtk_image_new_from_pixbuf (pixbuf);
  gtk_box_pack_start (GTK_BOX (hbox), album_art, FALSE, FALSE, 0);
  gtk_widget_show (album_art);
  */

  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  gtk_box_pack_start (GTK_BOX (self), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  self->label_title = label;
  garu_song_box_default_title (self);
}

static GtkWidget *
garu_song_box_init_progress_box_elements (GaruSongBox *self)
{
  GtkWidget *progress_box, *progress_bar, *position, *length, *event_box;

  progress_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
  /* Position */
  position = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (progress_box), position, FALSE, FALSE, 0);
  /* Progress bar */
  progress_bar = gtk_progress_bar_new ();
  event_box = gtk_event_box_new();
  gtk_container_add (GTK_CONTAINER (event_box), progress_bar);
  gtk_widget_set_valign (GTK_WIDGET(event_box), GTK_ALIGN_CENTER);
  gtk_box_pack_start (GTK_BOX (progress_box), event_box, TRUE, TRUE, 0);
  g_signal_connect (event_box, "button-press-event",
                    G_CALLBACK (garu_song_box_progress_bar_event_seek), self);
  /* Length */
  length = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (progress_box), length, FALSE, FALSE, 0);

  self->position = position;
  self->length = length;
  self->progress_bar = progress_bar;
  self->progress_box = progress_box;
}

static void
garu_song_box_progress_bar_event_seek (GtkWidget      *widget,
                                       GdkEventButton *event,
                                       GaruSongBox    *self)
{
  gdouble        fraction;
  GtkAllocation  allocation;
  GaruPlayer    *player;
  if (event->button != GDK_BUTTON_PRIMARY)
    return;
  player = garu_utils_get_player ();
  gtk_widget_get_allocation (widget, &allocation);
  fraction = (gdouble) event->x / allocation.width;
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (self->progress_bar),
                                 fraction);
  garu_player_set_position (player, fraction);
}

GtkWidget *
garu_song_box_new (void)
{
  return g_object_new (GARU_TYPE_SONG_BOX, NULL);
}

void
garu_song_box_progress_bar_set_fraction (GaruSongBox *self, gdouble fraction)
{
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (self->progress_bar),
                                 fraction);
}

double
garu_song_box_progress_bar_get_fraction (GaruSongBox *self)
{
  return gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (self->progress_bar));
}

void
garu_song_box_set_position (GaruSongBox *self, gint position)
{
  gchar *str, *text;
  str = garu_utils_convert_seconds (position);
  text = g_markup_printf_escaped ("<small>%s</small>", str);
  gtk_label_set_markup (GTK_LABEL (self->position), text);
  g_free (str);
  g_free (text);
}

void
garu_song_box_set_length(GaruSongBox *self, gint length)
{
  gchar *str, *text;
  str = garu_utils_convert_seconds (length);
  text = g_markup_printf_escaped ("<small>%s</small>", str);
  gtk_label_set_markup (GTK_LABEL (self->length), text);
  g_free (str);
  g_free (text);
}

void
garu_song_box_set_title (GaruSongBox *self, gchar *title, gchar *artist)
{
  gchar *text;
  if (artist == NULL)
    text = g_markup_printf_escaped ("<b>%s</b>", title);
  else
    text = g_markup_printf_escaped ("<b>%s</b> - %s", title, artist);
  gtk_label_set_markup (GTK_LABEL (self->label_title), text);
  g_free (text);
}

void
garu_song_box_default_title (GaruSongBox *self)
{
  gchar *garu;
  garu = garu_utils_text_bold ("Garu Music Player");
  gtk_label_set_markup (GTK_LABEL (self->label_title), garu);
  g_free (garu);
  gtk_widget_hide (self->progress_box);
  
}
