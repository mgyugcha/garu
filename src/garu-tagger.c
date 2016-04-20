/* garu-tagger.c
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

#include "garu-tagger.h"
#include "garu-utils.h"

struct _GaruTagger
{
  GObject parent;
  /* Tags */
  TagLib_File                  *tfile;
  TagLib_Tag                   *tag;
  const TagLib_AudioProperties *audio_prop;
  gchar *file;
};

G_DEFINE_TYPE (GaruTagger, garu_tagger, G_TYPE_OBJECT);

static void
garu_tagger_finalize (GObject *object)
{
  GaruTagger *self = GARU_TAGGER (object);
  g_free (self->file);
  taglib_file_free (self->tfile);
  G_OBJECT_CLASS (garu_tagger_parent_class)->finalize (object);
}

static void
garu_tagger_class_init (GaruTaggerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = garu_tagger_finalize;
}

static void
garu_tagger_init (GaruTagger *self)
{
  self->tfile = NULL;
  self->file = NULL;
}

GaruTagger *
garu_tagger_new (void)
{
  return g_object_new (GARU_TYPE_TAGGER, NULL);
}

gboolean
garu_tagger_set_uri (GaruTagger *self, gchar *uri)
{
  gchar   *file;
  gboolean response;
  file = g_filename_from_uri (uri, NULL, NULL);
  response = garu_tagger_set_file (self, file);
  g_free (file);
  return response;
}

gboolean
garu_tagger_set_file (GaruTagger *self, gchar *file)
{
#ifdef G_OS_WIN32
  GError *error = NULL;
  gchar  *encoded_file = g_locale_from_utf8 (file, -1, NULL, NULL, &err);
  if (encoded_file != NULL)
    {
      g_warning ("garu_tagger_set_file (%s): %s", file, error->message);
      g_error_free (error);
    }
  else
    {
      self->tfile = taglib_file_new (encoded_file);
      g_free (encoded_file);
    }
#else
  self->tfile = taglib_file_new (file);
#endif

  if (self->tfile == NULL)
    {
      g_warning ("Unable to open file using taglib : %s", file);
      return FALSE;
    }

  self->tag = taglib_file_tag (self->tfile);
  if (self->tag == NULL)
    {
      g_warning ("Unable to locate tag in file %s", file);
      return FALSE;
    }

  self->audio_prop = taglib_file_audioproperties (self->tfile);
  if (self->audio_prop == NULL)
    {
      g_warning("Unable to locate audio properties in file %s", file);
      return FALSE;
    }
  self->file = g_strdup (file);
  return TRUE;
}

gchar *
garu_tagger_get_title (GaruTagger *self)
{
  g_return_val_if_fail (self->tag, NULL);
  gchar *title;
  title = taglib_tag_title (self->tag);
  if (g_strcmp0 (title, "") == 0)
    title = g_path_get_basename (self->file);
  return title;
}

gchar *
garu_tagger_get_artist (GaruTagger *self)
{
  g_return_val_if_fail (self->tag, NULL);
  gchar *artist;
  artist = taglib_tag_artist (self->tag);
  if (g_strcmp0 (artist, "") == 0)
    {
      g_free (artist);
      artist = NULL;
    }
  return artist;
}

gchar *
garu_tagger_get_album (GaruTagger *self)
{
  g_return_val_if_fail (self->tag, NULL);
  gchar *album;
  album = taglib_tag_album (self->tag);
  if (g_strcmp0 (album, "") == 0)
    {
      g_free (album);
      album = NULL;
    }
  return album;
}

gchar *
garu_tagger_get_genre (GaruTagger *self)
{
  g_return_val_if_fail (self->tag, NULL);
  gchar *genre;
  genre = taglib_tag_genre (self->tag);
  if (g_strcmp0 (genre, "") == 0)
    {
      g_free (genre);
      genre = NULL;
    }
  return genre;
}

const gchar *
garu_tagger_get_file (GaruTagger *self)
{
  return self->file;
}

gint
garu_tagger_get_length (GaruTagger *self)
{
  g_return_val_if_fail (self->audio_prop, 0);
  return taglib_audioproperties_length (self->audio_prop);
}

gchar *
garu_tagger_get_length_str (GaruTagger *self)
{
  return garu_utils_convert_seconds (garu_tagger_get_length (self));
}

gchar *
garu_tagger_get_track_str (GaruTagger *self)
{
  g_return_val_if_fail (self->tag, NULL);
  gint track = taglib_tag_track (self->tag);
  if (track != 0)
    return g_strdup_printf ("%d", track);
  else
    return NULL;
}

gchar *
garu_tagger_get_year_str (GaruTagger *self)
{
  g_return_val_if_fail (self->tag, NULL);
  gint year = taglib_tag_year (self->tag);
  if (year != 0)
    return g_strdup_printf ("%d", year);
  else
    return NULL;
}
