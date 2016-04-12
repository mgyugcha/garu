/* garu-song-box.h
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

#ifndef __GARU_SONG_BOX_H__
#define __GARU_SONG_BOX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GARU_TYPE_SONG_BOX (garu_song_box_get_type())
G_DECLARE_FINAL_TYPE (GaruSongBox, garu_song_box, GARU, SONG_BOX, GtkBox);

struct _GaruSongBoxClass
{
  GtkBoxClass parent;
};

GtkWidget *garu_song_box_new                       (void);
void       garu_song_box_progress_bar_set_fraction (GaruSongBox *self,
                                                    gdouble fraction);
double     garu_song_box_progress_bar_get_fraction (GaruSongBox *self);
void       garu_song_box_set_position              (GaruSongBox *self,
                                                    gchar *position);
void       garu_song_box_set_length                (GaruSongBox *self,
                                                    gchar *length);
void       garu_song_box_set_label_title           (GaruSongBox *self,
                                                    gchar *title);
void       garu_song_box_default_title             (GaruSongBox *self);

G_END_DECLS

#endif /* __GARU_SONG_BOX_H__ */
