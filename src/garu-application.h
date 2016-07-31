/* garu-application.h
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

#ifndef __GARU_APPLICATION_H__
#define __GARU_APPLICATION_H__

#include <gtk/gtk.h>
#include "garu-player.h"
#include "garu-playlist.h"

G_BEGIN_DECLS

#define GARU_TYPE_APPLICATION (garu_application_get_type ())
G_DECLARE_FINAL_TYPE (GaruApplication, garu_application, GARU, APPLICATION, GtkApplication);

GaruApplication *garu_application_new                     (void);
GMenu           *garu_application_get_menu                (GaruApplication *self);
GaruPlayer      *garu_application_get_player              (GaruApplication *self);
void             garu_application_set_playlist            (GaruApplication *self,
                                                           GaruPlaylist *playlist);
GaruPlaylist    *garu_application_get_playlist            (GaruApplication *self);
GSettings       *garu_application_get_settings            (GaruApplication *self);
void             garu_application_show_message            (GaruApplication *self,
                                                           gchar           *message);

G_END_DECLS

#endif /* __GARU_APPLICATION_H__ */

