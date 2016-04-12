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

static void       garu_song_box_init_elements              (GaruSongBox *self);
static GtkWidget *garu_song_box_init_progress_box_elements (GaruSongBox *self);

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
  GtkWidget *hbox, *vbox, *progress_box, *label;//, *album_art;

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
  GtkWidget *progress_box, *progress_bar, *position, *length;

  progress_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
  /* Position */
  position = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (progress_box), position, FALSE, FALSE, 0);
  /* Progress bar */
  progress_bar = gtk_progress_bar_new ();
  gtk_widget_set_valign (GTK_WIDGET(progress_bar), GTK_ALIGN_CENTER);
  gtk_box_pack_start (GTK_BOX (progress_box), progress_bar, TRUE, TRUE, 0);     
  /* Length */
  length = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (progress_box), length, FALSE, FALSE, 0);

  self->position = position;
  self->length = length;
  self->progress_bar = progress_bar;
  self->progress_box = progress_box;
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
garu_song_box_set_position (GaruSongBox *self, gchar *position)
{
  gtk_label_set_markup (GTK_LABEL (self->position), position);
}

void
garu_song_box_set_length(GaruSongBox *self, gchar *length)
{
  gtk_label_set_markup (GTK_LABEL (self->length), length);
}

void
garu_song_box_set_label_title (GaruSongBox *self, gchar *title)
{
  gtk_label_set_markup (GTK_LABEL (self->label_title), title);
}

void
garu_song_box_default_title (GaruSongBox *self)
{
  gtk_label_set_markup (GTK_LABEL (self->label_title),
                        "<b>Garu Music Player</b>");
  gtk_widget_hide (self->progress_box);
  
}
