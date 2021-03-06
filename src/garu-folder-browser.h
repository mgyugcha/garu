/* garu-folder-browser.h
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

#ifndef GARU_FOLDER_BROWSER_H
#define GARU_FOLDER_BROWSER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GARU_TYPE_FOLDER_BROWSER (garu_folder_browser_get_type())

G_DECLARE_FINAL_TYPE (GaruFolderBrowser, garu_folder_browser, GARU, FOLDER_BROWSER, GtkBox);

struct _GaruFolderBrowserClass
{
  GtkScrolledWindowClass parent;
};

GtkWidget *garu_folder_browser_new (void);

G_END_DECLS

#endif /* GARU_FOLDER_BROWSER_H */
