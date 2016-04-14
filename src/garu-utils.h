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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

/* CONSTANTS */
#define GARU_CELL_PADDING 4

enum {
  TARGET_REF_LIBRARY
};

static const gchar *eq_bands[] =
  {
    "32", "64", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"
  };

static const gchar *eq_presets[] =
  {
    N_("Normal"),
    N_("Ballad"),
    N_("Classic"),
    N_("Club"),
    N_("Dance"),
    N_("Pop"),
    N_("Reggae"),
    N_("Rock"),
    N_("Ska"),
    N_("Soft"),
    N_("Party"),
    N_("More Bass"),
    N_("More Bass and Treble"),
    N_("Custom")
  };

static const gdouble
eq_presets_values[G_N_ELEMENTS (eq_presets)][G_N_ELEMENTS (eq_bands)] =
  {
    {  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00 }, // Normal
    {  4.00,  3.75,  2.50,  0.00, -4.00, -6.00, -3.00,  0.00,  2.50,  9.00 }, // Ballad
    {  0.00,  0.00,  0.00,  0.00,  0.00,  0.00, -6.00, -7.00, -7.00, -9.50 }, // Classic
    {  0.00,  0.00,  8.00,  6.00,  5.50,  5.00,  3.00,  0.00,  0.00,  0.00 }, // Club
    {  9.60,  7.00,  2.50,  0.00,  0.00, -5.60, -7.00, -7.00,  0.00,  0.00 }, // Dance
    { -1.60,  4.50,  7.00,  8.00,  5.60,  0.00, -2.50, -2.00, -1.60, -1.50 }, // Pop
    {  0.00,  0.00,  0.00, -5.50,  0.00,  6.50,  6.50,  0.00,  0.00,  0.00 }, // Reggae
    {  8.00,  5.00, -5.50, -8.00, -3.00,  4.00,  8.00, 11.00, 11.00, 11.50 }, // Rock
    { -2.50, -5.00, -4.00,  0.00,  4.00,  5.50,  8.00,  9.00, 11.00,  9.00 }, // Ska
    {  5.00,  1.50,  0.00, -2.50,  0.00,  4.00,  8.00,  9.00, 11.00, 12.00 }, // Soft
    {  7.00,  7.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  7.00,  7.00 }, // Party
    { -8.00, 10.00, 10.00,  5.50,  1.50, -4.00, -8.00,-10.00,-11.00,-11.00 }, // More Bass
    {  8.00,  5.50,  0.00, -7.00, -5.00,  1.50,  8.00, 11.20, 12.00, 12.00 }, // More Bass and Treble
  };


gchar   *garu_utils_convert_seconds (gint length);
gboolean garu_utils_is_audio        (gchar *path);
gboolean garu_utils_is_hidden       (gchar *path);
void     garu_utils_free_string     (GString *string);
gint     garu_utils_g_strcmp        (GString *a, GString *b);

#endif // __GARU_UTILS_H__
