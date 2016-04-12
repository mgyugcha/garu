/* garu-player-bin.h
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

#ifndef __GARU_PLAYER_BIN_H__
#define __GARU_PLAYER_BIN_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GARU_TYPE_PLAYER_BIN (garu_player_bin_get_type ())
G_DECLARE_FINAL_TYPE (GaruPlayerBin, garu_player_bin, GARU, PLAYER_BIN, GObject);

struct _GaruPlayerBinClass
{
  GObjectClass parent;
};

GaruPlayerBin *garu_player_bin_new          (void);
void           garu_player_bin_set_uri      (GaruPlayerBin *self, gchar *uri);
void           garu_player_bin_play         (GaruPlayerBin *self);
void           garu_player_bin_pause        (GaruPlayerBin *self);
void           garu_player_bin_stop         (GaruPlayerBin *self);
void           garu_player_bin_set_volume   (GaruPlayerBin *self,
					     gdouble value);
gint64         garu_player_bin_get_position (GaruPlayerBin *self);
gint64         garu_player_bin_get_duration (GaruPlayerBin *self);

G_END_DECLS

#endif // __GARU_PLAYER_BIN_H__
