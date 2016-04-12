/* garu-effects.c
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

#include "garu-effects.h"

struct _GaruEffects
{
  GtkDialog parent_instance;
};

G_DEFINE_TYPE (GaruEffects, garu_effects, GTK_TYPE_DIALOG);

enum {
  PROP_0,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void garu_effects_init_container      (GaruEffects *self);
static GtkWidget *garu_effects_get_equalizer (void);

static void
garu_effects_finalize (GObject *object)
{
  GaruEffects *self = (GaruEffects *)object;

  G_OBJECT_CLASS (garu_effects_parent_class)->finalize (object);
}

static void
garu_effects_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  GaruEffects *self = GARU_EFFECTS (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
garu_effects_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  GaruEffects *self = GARU_EFFECTS (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
garu_effects_class_init (GaruEffectsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = garu_effects_finalize;
  object_class->get_property = garu_effects_get_property;
  object_class->set_property = garu_effects_set_property;
}

static void
garu_effects_init (GaruEffects *self)
{
  gtk_window_set_title (GTK_WINDOW (self), _("Effects"));
  
  garu_effects_init_container (self);
}

static void
garu_effects_init_container (GaruEffects *self)
{
  GtkWidget *grid, *label, *toogle, *box, *scale, *equalizer;

  grid = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 10);
  gtk_widget_set_halign (GTK_WIDGET (grid), GTK_ALIGN_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (grid), 10);

  /* Equalizer */
  label = gtk_label_new (_("Equalizer"));
  toogle = gtk_switch_new ();
  equalizer = garu_effects_get_equalizer ();
  gtk_grid_attach (GTK_GRID (grid), label, 1, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), toogle, 2, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), equalizer, 1, 2, 2, 1);

  /* /\* Echo *\/ */
  /* label = gtk_label_new (_("Echo")); */
  /* toogle = gtk_switch_new (); */
  /* scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.1); */
  /* gtk_grid_attach (GTK_GRID (grid), label, 1, 2, 1, 1); */
  /* gtk_grid_attach (GTK_GRID (grid), toogle, 2, 2, 1, 1); */
  /* gtk_grid_attach (GTK_GRID (grid), scale, 1, 3, 2, 1); */

  /* /\* Mono channel *\/ */
  /* label = gtk_label_new (_("Use mono channel")); */
  /* toogle = gtk_switch_new (); */
  /* gtk_grid_attach (GTK_GRID (grid), label, 1, 4, 1, 1); */
  /* gtk_grid_attach (GTK_GRID (grid), toogle, 2, 4, 1, 1); */

  box = gtk_dialog_get_content_area (GTK_DIALOG (self));
  gtk_container_set_border_width (GTK_CONTAINER (box), 0);
  gtk_box_pack_start (GTK_BOX (box), grid, TRUE, TRUE, 0);
  gtk_widget_show_all (box);
}

static GtkWidget *
garu_effects_get_equalizer (void)
{
  const gint bands = 10;
  gint i;
  GtkWidget *grid, *scale, *combo_box;

  grid = gtk_grid_new ();
  combo_box = gtk_combo_box_text_new ();
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combo_box), NULL, "Normal");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combo_box), NULL, "Custom");

  gtk_grid_attach (GTK_GRID(grid), combo_box, 1, 1, 2, 1);

  for (i = 0; i < bands; i++)
    {
      scale = gtk_scale_new_with_range (GTK_ORIENTATION_VERTICAL, -24, 12, 1);
      gtk_widget_set_vexpand (scale, TRUE);
      gtk_grid_attach (GTK_GRID(grid), scale, i, 2, 1, 3);
    }
  

  return grid;
}

GaruEffects *
garu_effects_new (GtkWindow *window)
{
  return g_object_new (GARU_TYPE_EFFECTS,
		       "modal", TRUE,
		       "transient-for", window,
		       "use-header-bar", TRUE, NULL);
}
