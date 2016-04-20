/* garu-utils.c
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

#include "garu-application.h"
#include "garu-utils.h"

gchar *
garu_utils_convert_seconds (gint length)
{
  gchar *str;
  gint   hours = 0, minutes = 0, seconds = 0;

  if (length > 3600)
    {
      hours = length / 3600;
      length = length % 3600;
    }

  if (length > 60)
    {
      minutes = length / 60;
      seconds = length % 60;
    }
  else
    seconds = length;

  if (hours == 0)
    str = g_strdup_printf ("%02d:%02d", minutes, seconds);
  else
    str = g_strdup_printf ("%d:%02d:%02d", hours, minutes, seconds);
  return str;
}


gboolean
garu_utils_is_audio (gchar *path)
{
  GFile       *file;
  GFileInfo   *info;
  const gchar *content_type;
  gboolean     response = FALSE;
  file = g_file_new_for_path (path);
  info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
			    G_FILE_QUERY_INFO_NONE, NULL, NULL);
  content_type = g_file_info_get_content_type (info);
  response = g_strrstr (content_type, "audio/") != NULL;
  g_object_unref (file);
  g_object_unref (info);
  return response;
}

gboolean
garu_utils_is_hidden (gchar *path)
{
  GFile     *file;
  GFileInfo *info;
  gboolean   response = FALSE;
  file = g_file_new_for_path (path);
  info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN,
			    G_FILE_QUERY_INFO_NONE, NULL, NULL);
  response = g_file_info_get_is_hidden (info);
  g_object_unref (file);
  g_object_unref (info);
  return response;
}

void
garu_utils_free_string (GString *string)
{
  g_string_free (string, TRUE);
}

gint
garu_utils_g_strcmp (GString *a, GString *b)
{
  return g_strcmp0 (a->str, b->str);
}

gchar *
garu_utils_text_bold (gchar *text)
{
  return g_markup_printf_escaped ("<b>%s</b>", text);
}

GSettings *
garu_utils_get_settings (void)
{
  GSettings       *settings;
  GaruApplication *app;

  app = GARU_APPLICATION (g_application_get_default ());
  settings = garu_application_get_settings (app);

  return settings;
}
