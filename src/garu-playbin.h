/* garu-playbin.h
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

#ifndef __GARU_PLAYBIN_H__
#define __GARU_PLAYBIN_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GARU_TYPE_PLAYBIN (garu_playbin_get_type ())
G_DECLARE_FINAL_TYPE (GaruPlaybin, garu_playbin, GARU, PLAYBIN, GObject);

GaruPlaybin   *garu_playbin_new                (void);
void           garu_playbin_set_uri            (GaruPlaybin *self,
						gchar       *uri);
void           garu_playbin_play               (GaruPlaybin *self);
void           garu_playbin_pause              (GaruPlaybin *self);
void           garu_playbin_stop               (GaruPlaybin *self);
void           garu_playbin_set_volume         (GaruPlaybin *self,
						gdouble      value);
void           garu_playbin_set_position       (GaruPlaybin *self,
						gdouble      fraction);
gint64         garu_playbin_get_position       (GaruPlaybin *self);
gint64         garu_playbin_get_duration       (GaruPlaybin *self);
void           garu_playbin_equalizer_disabled (GaruPlaybin *self);
GstElement    *garu_playbin_get_equalizer      (GaruPlaybin *self);
void           garu_playbin_disable_equalizer  (GaruPlaybin *self);
void           garu_playbin_update_equalizer   (GaruPlaybin *self);

G_END_DECLS

#endif // __GARU_PLAYBIN_H__
