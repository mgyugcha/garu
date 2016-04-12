/* garu-playlist.h
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

#ifndef __GARU_PLAYLIST_H__
#define __GARU_PLAYLIST_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GARU_TYPE_PLAYLIST (garu_playlist_get_type())

G_DECLARE_FINAL_TYPE (GaruPlaylist, garu_playlist, GARU, PLAYLIST, GtkBox);

struct _GaruPlaylistClass
{
  GtkBoxClass parent;
};

GaruPlaylist *garu_playlist_new        (void);
gchar        *garu_playlist_get_track (GaruPlaylist *self);
gchar        *garu_playlist_get_next_track (GaruPlaylist *self);
gchar        *garu_playlist_get_previous_track (GaruPlaylist *self);

G_END_DECLS

#endif /* __GARU_PLAYLIST_H__ */
