/* garu-window.c
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

#include "garu-window.h"
#include "garu-player.h"
#include "garu-library.h"
#include "garu-playlist.h"

struct _GaruWindow
{
  GtkApplicationWindow parent;

  GtkWidget           *headerbar;
  GaruPlayer          *player;
};

G_DEFINE_TYPE (GaruWindow, garu_window, GTK_TYPE_APPLICATION_WINDOW);

static void garu_window_init_headerbar (GaruWindow *self);
static void garu_window_init_container (GaruWindow *self);
static void garu_window_init_player (GaruWindow *self);

static void
garu_window_init (GaruWindow *self)
{
  garu_window_init_headerbar (self);
  garu_window_init_container (self);
  gtk_window_set_default_size (GTK_WINDOW (self), 900, 600);
}

static void
garu_window_class_init (GaruWindowClass *klass)
{
}

static void
garu_window_init_headerbar (GaruWindow *self)
{
  GtkWidget *headerbar;

  headerbar = garu_headerbar_new ();
  gtk_window_set_titlebar (GTK_WINDOW (self), headerbar);
  gtk_widget_show (headerbar);
  self->headerbar = headerbar;
}

static void
garu_window_init_container (GaruWindow *self)
{
  GtkWidget       *box, *paned;
  GaruPlaylist    *playlist;
  GaruLibrary     *library;
  GaruApplication *app;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_show (paned);
  gtk_box_pack_start (GTK_BOX (box), paned, TRUE, TRUE, 0);

  library = garu_library_new ();
  gtk_widget_show_all (GTK_WIDGET (library));
  gtk_paned_pack1 (GTK_PANED (paned), GTK_WIDGET (library), FALSE, FALSE);
  
  playlist = garu_playlist_new ();
  gtk_widget_show (GTK_WIDGET (playlist));
  gtk_paned_pack2 (GTK_PANED (paned), GTK_WIDGET (playlist), TRUE, FALSE);

  gtk_container_add (GTK_CONTAINER (self), box);

  gtk_widget_show (box);

  app = GARU_APPLICATION (g_application_get_default ());
  garu_application_set_playlist (app, playlist);
  g_object_unref (playlist);
}

static void
garu_window_init_player (GaruWindow *self)
{
  self->player = garu_player_new ();
}

GaruWindow *
garu_window_new (GaruApplication *app)
{
  return g_object_new (GARU_TYPE_WINDOW, "application", app, NULL);
}
