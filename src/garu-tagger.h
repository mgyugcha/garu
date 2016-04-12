/* garu-tagger.h
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

#ifndef __GARU_TAGGER_H__
#define __GARU_TAGGER_H__

#include <tag_c.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GARU_TYPE_TAGGER (garu_tagger_get_type ())
G_DECLARE_FINAL_TYPE (GaruTagger, garu_tagger, GARU, TAGGER, GObject);

struct _GaruTaggerClass
{
  GObjectClass parent;
};

GaruTagger  *garu_tagger_new            (void);
gboolean     garu_tagger_set_uri        (GaruTagger *self, gchar *uri);
gboolean     garu_tagger_set_file       (GaruTagger *self, gchar *file);
gchar       *garu_tagger_get_title      (GaruTagger *self);
gchar       *garu_tagger_get_artist     (GaruTagger *self);
gint         garu_tagger_get_length     (GaruTagger *self);
gchar       *garu_tagger_get_length_str (GaruTagger *self);
gchar       *garu_tagger_get_album      (GaruTagger *self);
gchar       *garu_tagger_get_genre      (GaruTagger *self);
const gchar *garu_tagger_get_file       (GaruTagger *self);
gchar       *garu_tagger_get_track_str  (GaruTagger *self);
gchar       *garu_tagger_get_year_str   (GaruTagger *self);

G_END_DECLS

#endif // __GARU_TAGGER_H__
