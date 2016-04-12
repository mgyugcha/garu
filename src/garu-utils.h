/* garu-utils.h
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

#ifndef __GARU_UTILS_H__
#define __GARU_UTILS_H__

#include <gtk/gtk.h>

/* CONSTANTS */
#define GARU_CELL_PADDING 4

enum {
  TARGET_REF_LIBRARY
};

gchar   *garu_utils_convert_seconds (gint length);
gboolean garu_utils_is_audio        (gchar *path);
gboolean garu_utils_is_hidden       (gchar *path);
void     garu_utils_free_string     (GString *string);
gint     garu_utils_g_strcmp        (GString *a, GString *b);

#endif // __GARU_UTILS_H__
