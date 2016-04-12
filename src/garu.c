/* garu.c
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

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <glib/gi18n.h>
#include <config.h>
#include "garu-application.h"

gint
main (gint argc, gchar *argv[])
{
  GaruApplication *app;
  gst_init (&argc, &argv);

  /* Gettext */
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);

  app = garu_application_new ();
  return g_application_run (G_APPLICATION (app), argc, argv);
}
