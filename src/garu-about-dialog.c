/* garu-about-dialog.c
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

#include "garu-about-dialog.h"

struct _GaruAboutDialog
{
  GtkAboutDialog parent_instance;
};

G_DEFINE_TYPE (GaruAboutDialog, garu_about_dialog, GTK_TYPE_ABOUT_DIALOG)

static void
garu_about_dialog_class_init (GaruAboutDialogClass *klass)
{
}

static void
garu_about_dialog_init (GaruAboutDialog *self)
{
  GtkAboutDialog *about = GTK_ABOUT_DIALOG (self);
  const gchar *authors[] = {
    "Michael Yugcha",
    NULL
  };

  gtk_about_dialog_set_program_name (about, "Garu Music Player");
  gtk_about_dialog_set_version (about, "1.0.0");
  gtk_about_dialog_set_copyright (about, "Copyright 2016 - Michael Yugcha");
  gtk_about_dialog_set_license_type (about, GTK_LICENSE_GPL_3_0);
  gtk_about_dialog_set_comments (about, "Garu, en donde quiera que estes.");
  gtk_about_dialog_set_authors (about, authors);
}

GaruAboutDialog *
garu_about_dialog_new (GtkWindow *window)
{
  return g_object_new (GARU_TYPE_ABOUT_DIALOG,
		       "modal", TRUE,
		       "transient-for", window,
		       "use-header-bar", TRUE, NULL);
}
