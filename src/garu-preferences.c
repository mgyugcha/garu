/* garu-preferences.c
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
#include "garu-preferences.h"

struct _GaruPreferences
{
  GtkDialog  parent_instance;
  GSettings *settings;
};

G_DEFINE_TYPE (GaruPreferences, garu_preferences, GTK_TYPE_DIALOG);

enum {
  PROP_0,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void       garu_preferences_init_container      (GaruPreferences *self);
static GtkWidget *garu_preferences_init_box_appearance (GaruPreferences *self);

static void
garu_preferences_finalize (GObject *object)
{
  GaruPreferences *self = GARU_PREFERENCES (object);

  G_OBJECT_CLASS (garu_preferences_parent_class)->finalize (object);
}

static void
garu_preferences_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  GaruPreferences *self = GARU_PREFERENCES (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
garu_preferences_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  GaruPreferences *self = GARU_PREFERENCES (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
garu_preferences_class_init (GaruPreferencesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = garu_preferences_finalize;
  object_class->get_property = garu_preferences_get_property;
  object_class->set_property = garu_preferences_set_property;
}

static void
garu_preferences_init (GaruPreferences *self)
{
  self->settings = g_settings_new ("com.github.mgyugcha.garu");
  gtk_window_set_title (GTK_WINDOW (self), _("Preferences"));
  garu_preferences_init_container (self);
}

static void
garu_preferences_init_container (GaruPreferences *self)
{
  GtkWidget *notebook, *box, *label;
  GtkWidget *appearance;

  notebook = gtk_notebook_new ();
  box = gtk_dialog_get_content_area (GTK_DIALOG (self));
  gtk_container_set_border_width (GTK_CONTAINER (box), 0);

  /* Appearance */
  label = gtk_label_new (_("Appearance"));
  appearance = garu_preferences_init_box_appearance (self);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), appearance, label);

  gtk_box_pack_start (GTK_BOX (box), notebook, TRUE, TRUE, 0);
  gtk_widget_show_all (box);
}

static GtkWidget *
garu_preferences_init_box_appearance (GaruPreferences *self)
{
  GaruApplication *app;
  GtkWidget       *grid, *label, *toogle;

  app = GARU_APPLICATION (g_application_get_default ());

  grid = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 10);
  gtk_widget_set_halign (GTK_WIDGET (grid), GTK_ALIGN_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (grid), 10);

  /* dark mode */
  label = gtk_label_new (_("Use dark theme"));
  toogle = gtk_switch_new ();
  g_settings_bind (self->settings, "use-dark-theme",
                   toogle, "active", G_SETTINGS_BIND_DEFAULT);
  gtk_grid_attach (GTK_GRID (grid), label, 1, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), toogle, 2, 1, 1, 1);

  /* preferences button */
  if (gtk_application_prefers_app_menu (GTK_APPLICATION (app)))
    {
      label = gtk_label_new (_("Show button preferences in the window"));
      toogle = gtk_switch_new ();
      g_settings_bind (self->settings, "show-settings-button",
                       toogle, "active", G_SETTINGS_BIND_DEFAULT);
      gtk_grid_attach (GTK_GRID (grid), label, 1, 2, 1, 1);
      gtk_grid_attach (GTK_GRID (grid), toogle, 2, 2, 1, 1);

    }
  return grid;
}

GaruPreferences *
garu_preferences_new (GtkWindow *window)
{
  return g_object_new (GARU_TYPE_PREFERENCES,
                       "modal", TRUE,
                       "transient-for", window,
                       "use-header-bar", TRUE, NULL);
}
