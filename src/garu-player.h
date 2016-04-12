/* garu-player.h
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

#ifndef __GARU_PLAYER_H__
#define __GARU_PLAYER_H__

#include <glib-object.h>

#include "garu-tagger.h"

G_BEGIN_DECLS

#define GARU_TYPE_PLAYER (garu_player_get_type())
G_DECLARE_FINAL_TYPE (GaruPlayer, garu_player, GARU, PLAYER, GObject);

struct _GaruPlayerClass
{
  GObjectClass parent;
};

enum {
  GARU_PLAYER_PLAYING,
  GARU_PLAYER_PAUSED,
  GARU_PLAYER_STOPPED,
  GARU_PLAYER_CROSSFADING
};

GaruPlayer *garu_player_new              (void);
void        garu_player_set_track        (GaruPlayer *self, gchar *uri);
void        garu_player_play             (GaruPlayer *self);
void        garu_player_pause            (GaruPlayer *self);
void        garu_player_stop             (GaruPlayer *self);
gint        garu_player_get_status       (GaruPlayer *self);
void        garu_player_set_volume       (GaruPlayer *self, gdouble value);
gint64      garu_player_get_position     (GaruPlayer *self);
gchar      *garu_player_get_position_str (GaruPlayer *self);
GaruTagger *garu_player_get_tagger       (GaruPlayer *self);

G_END_DECLS

#endif /* __GARU_PLAYER_H__ */