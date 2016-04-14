/* garu-application.c
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

#include <glib/gi18n.h>

#include "garu-application.h"
#include "garu-about-dialog.h"
#include "garu-effects.h"
#include "garu-preferences.h"
#include "garu-window.h"

struct _GaruApplication
{
  GtkApplication parent;

  GaruPlayer    *player;
  GaruPlaylist  *playlist;
  GSettings     *settings;
};

G_DEFINE_TYPE (GaruApplication, garu_application, GTK_TYPE_APPLICATION);

static void garu_application_effects_activated     (GSimpleAction *action,
						    GVariant      *parameter,
						    gpointer       self);
static void garu_application_preferences_activated (GSimpleAction *action,
						    GVariant      *parameter,
						    gpointer       self);
static void garu_application_about_activated       (GSimpleAction *action,
						    GVariant      *parameter,
						    gpointer       self);
static void garu_application_quit_activated        (GSimpleAction *action,
						    GVariant      *parameter,
						    gpointer       self);

static GActionEntry app_entries[] =
  {
    { "effects", garu_application_effects_activated, NULL, NULL, NULL },
    { "preferences", garu_application_preferences_activated, NULL, NULL, NULL },
    { "about", garu_application_about_activated, NULL, NULL, NULL },
    { "quit", garu_application_quit_activated, NULL, NULL, NULL }
  };

static void
garu_application_activate (GApplication *self)
{
  GaruWindow *window;

  GARU_APPLICATION (self)->player = garu_player_new ();

  window = garu_window_new (GARU_APPLICATION (self));
  gtk_window_present (GTK_WINDOW (window));
}

static void
garu_application_startup (GApplication *self)
{
  GMenu *menu;

  G_APPLICATION_CLASS (garu_application_parent_class)->startup (self);

  g_action_map_add_action_entries (G_ACTION_MAP (self), app_entries,
				   G_N_ELEMENTS (app_entries), self);

  menu = garu_application_get_menu (GARU_APPLICATION (self));
  gtk_application_set_app_menu (GTK_APPLICATION (self), G_MENU_MODEL (menu));

  
  /* Dark Theme */
  GtkSettings *gtk_settings = gtk_settings_get_default ();
  g_settings_bind (GARU_APPLICATION (self)->settings, "dark-theme",
		   gtk_settings, "gtk-application-prefer-dark-theme",
		   G_SETTINGS_BIND_DEFAULT);
}

static void
garu_application_class_init (GaruApplicationClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  app_class->activate = garu_application_activate;
  app_class->startup = garu_application_startup;
}

static void
garu_application_init (GaruApplication *self)
{
  self->player = NULL;
  self->playlist = NULL;
  self->settings = g_settings_new ("com.github.mgyugcha.garu");
}

static void
garu_application_effects_activated (GSimpleAction *action,
				    GVariant      *parameter,
				    gpointer       self)
{
  GaruEffects *effects;
  GtkWindow   *window;
  window = gtk_application_get_active_window (GTK_APPLICATION (self));
  effects = garu_effects_new (window);
  gtk_window_present (GTK_WINDOW (effects));
}

static void
garu_application_preferences_activated (GSimpleAction *action,
					GVariant      *parameter,
					gpointer       self)
{
  GaruPreferences *preferences;
  GtkWindow       *window;
  window = gtk_application_get_active_window (GTK_APPLICATION (self));
  preferences = garu_preferences_new (window);
  gtk_window_present (GTK_WINDOW (preferences));
}

static void
garu_application_about_activated (GSimpleAction *action,
				  GVariant      *parameter,
				  gpointer       self)
{
  GaruAboutDialog *about;
  GtkWindow       *window;
  window = gtk_application_get_active_window (GTK_APPLICATION (self));
  about = garu_about_dialog_new (window);
  gtk_window_present (GTK_WINDOW (about));
}

static void
garu_application_quit_activated (GSimpleAction *action,
				 GVariant      *parameter,
				 gpointer       self)
{
  g_application_quit (G_APPLICATION (self));
}

GaruApplication *
garu_application_new (void)
{
  return g_object_new (GARU_TYPE_APPLICATION,
		       "application-id", "com.github.mgyugcha.garu",
		       NULL);
}

GMenu *
garu_application_get_menu (GaruApplication *self)
{
  GMenu *menu, *view;

  menu = g_menu_new ();
  view = g_menu_new ();

  g_menu_append_submenu (menu, "Show", G_MENU_MODEL (view));
  g_menu_append (view, _("Lateral panel"), NULL);
  g_menu_append (view, _("Status bar"), NULL);
  g_menu_append (menu, _("Effects"), "app.effects");
  g_menu_append (menu, _("Preferences"), "app.preferences");
  g_menu_append (menu, _("About"), "app.about");
  g_menu_append (menu, _("Quit"), "app.quit");

  return menu;
}

GaruPlayer *
garu_application_get_player (GaruApplication *self)
{
  return self->player;
}

void
garu_application_set_playlist (GaruApplication *self, GaruPlaylist *playlist)
{
  self->playlist = g_object_ref (playlist);
}

GaruPlaylist *
garu_application_get_playlist (GaruApplication *self)
{
  return self->playlist;
}

GSettings *
garu_application_get_settings (GaruApplication *self)
{
  return self->settings;
}
