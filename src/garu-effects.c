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

#include "garu-application.h"
#include "garu-effects.h"
#include "garu-utils.h"

struct _GaruEffects
{
  GtkDialog parent_instance;
};

G_DEFINE_TYPE (GaruEffects, garu_effects, GTK_TYPE_DIALOG);

enum {
  PROP_0,
  N_PROPS
};

static gboolean equalizer_custom = TRUE;

static GParamSpec *properties [N_PROPS];

static void garu_effects_init_container      (GaruEffects *self);
static GtkWidget *garu_effects_init_box_equalizer (void);
static void garu_effects_equalizer_set_range (GtkRange  *range,
					      GPtrArray *data);
static void garu_effects_equalizer_set_combo_box (GtkComboBox *widget, GSList *scales);

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
  gtk_window_set_default_size (GTK_WINDOW (self), -1, 350);
  
  garu_effects_init_container (self);
}

static void
garu_effects_init_container (GaruEffects *self)
{
  GtkWidget *notebook, *box, *label;
  GtkWidget *equalizer;

  notebook = gtk_notebook_new ();
  box = gtk_dialog_get_content_area (GTK_DIALOG (self));
  gtk_container_set_border_width (GTK_CONTAINER (box), 0);

  label = gtk_label_new (_("Equalizer"));
  equalizer = garu_effects_init_box_equalizer ();
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), equalizer, label);

  /* label = gtk_label_new (_("Karaoke")); */
  /* equalizer = garu_effects_init_box_equalizer (); */
  /* gtk_notebook_append_page (GTK_NOTEBOOK (notebook), equalizer, label); */

  /* label = gtk_label_new (_("Crosfade")); */
  /* equalizer = garu_effects_init_box_equalizer (); */
  /* gtk_notebook_append_page (GTK_NOTEBOOK (notebook), equalizer, label); */

  /* label = gtk_label_new (_("Others")); */
  /* equalizer = garu_effects_init_box_equalizer (); */
  /* gtk_notebook_append_page (GTK_NOTEBOOK (notebook), equalizer, label); */

  gtk_box_pack_start (GTK_BOX (box), notebook, TRUE, TRUE, 0);
  gtk_widget_show_all (box);
}

static GtkWidget *
garu_effects_init_box_equalizer (void)
{
  gint i;
  GtkWidget *vbox, *hbox, *grid, *label, *toogle, *combo_box, *scale;
  GSList *list = NULL;
  GSettings *settings;
  GaruApplication *app;

  app = GARU_APPLICATION (g_application_get_default ());
  settings = garu_application_get_settings (app);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
  gtk_widget_set_halign (vbox, GTK_ALIGN_CENTER);
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);

  /* Label enabled */
  label = gtk_label_new (_("Enabled"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  toogle = gtk_switch_new ();
  gtk_box_pack_start (GTK_BOX (hbox), toogle, FALSE, FALSE, 0);
  /* ComboBox Presets */
  label = gtk_label_new (_("Presets"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  combo_box = gtk_combo_box_text_new ();
  for (i = 0 ; i < G_N_ELEMENTS (eq_presets); i++)
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo_box),
				    eq_presets[i]);
  gtk_box_pack_start (GTK_BOX (hbox), combo_box, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  /* Grid equalizer */
  grid = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID (grid), 0);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 0);
  gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);

  /* Equalizer bands */
  for (i = 0; i < G_N_ELEMENTS (eq_bands); i++)
    {
      GPtrArray *array;
      gchar     *band = g_strdup_printf ("eq-custom-band%d", i);
      scale = gtk_scale_new_with_range (GTK_ORIENTATION_VERTICAL,
					-12, 12, 0.1);
      label = gtk_label_new (eq_bands [i]);
      gtk_range_set_inverted (GTK_RANGE (scale), TRUE);
      gtk_scale_add_mark (GTK_SCALE (scale),  12.0, GTK_POS_LEFT, NULL);
      gtk_scale_add_mark (GTK_SCALE (scale),   0.0, GTK_POS_LEFT, NULL);
      gtk_scale_add_mark (GTK_SCALE (scale), -12.0, GTK_POS_LEFT, NULL);
      gtk_widget_set_vexpand (scale, TRUE);
      gtk_grid_attach (GTK_GRID (grid), scale, i, 0, 1, 1);
      gtk_grid_attach (GTK_GRID (grid), label, i, 1, 1, 1);
      /* Array */
      array = g_ptr_array_new ();
      g_ptr_array_add (array, combo_box);
      g_ptr_array_add (array, band);
      g_signal_connect (scale, "value-changed",
			(GCallback) garu_effects_equalizer_set_range, array);
      list = g_slist_append (list, scale);
    }

  g_signal_connect (combo_box, "changed",
		    (GCallback) garu_effects_equalizer_set_combo_box, list);
  gtk_box_pack_start (GTK_BOX (vbox), grid, TRUE, TRUE, 0);

  /* Bind */
  g_settings_bind (settings, "equalizer-enabled", toogle, "active",
		   G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (settings, "equalizer-enabled", label, "sensitive",
		   G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "equalizer-enabled", grid, "sensitive",
		   G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "equalizer-preset", combo_box, "active",
		   G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "equalizer-enabled", combo_box, "sensitive",
		   G_SETTINGS_BIND_GET);

  return vbox;
}

static void
garu_effects_equalizer_set_range (GtkRange  *range,
				  GPtrArray *data)
{
  GaruApplication *app;
  GaruPlayer      *player;
  GSettings       *settings;
  gint             active, i;
  GtkComboBox     *combo_box = g_ptr_array_index (data, 0);
  gchar           *band = g_ptr_array_index (data, 1);

  app = GARU_APPLICATION (g_application_get_default ());
  settings = garu_application_get_settings (app);
  player = garu_application_get_player (app);

  if (equalizer_custom)
    {
      active = gtk_combo_box_get_active (combo_box);
      if (active == G_N_ELEMENTS (eq_presets) - 1)
	{
	  g_settings_set_double (settings, band, gtk_range_get_value (range));
	}
      else
	{
	  for (i = 0; i < G_N_ELEMENTS (eq_bands); i++)
	    {
	      gchar *settings_band = g_strdup_printf ("eq-custom-band%d", i);
	      g_settings_set_double (settings, settings_band,
				     eq_presets_values[active][i]);
	      g_free (settings_band);
	    }
	  gtk_combo_box_set_active (combo_box, G_N_ELEMENTS (eq_presets) - 1);
	}
    }
  garu_player_update_equalizer (player);
}

static void
garu_effects_equalizer_set_combo_box (GtkComboBox *widget, GSList *scales)
{
  GaruApplication *app;
  GSettings       *settings;
  int              active, i;
  GSList          *l;

  app = GARU_APPLICATION (g_application_get_default ());
  settings = garu_application_get_settings (app);

  active = gtk_combo_box_get_active (widget);
  g_settings_set_int (settings, "equalizer-preset", active);

  equalizer_custom = FALSE;
  for (l = scales, i = 0; i < G_N_ELEMENTS (eq_bands); i++, l = l->next)
    {
      if (active == G_N_ELEMENTS (eq_presets) - 1) /* Custom */
	{
	  gchar *band = g_strdup_printf ("eq-custom-band%d", i);
	  gtk_range_set_value (GTK_RANGE (l->data),
			       g_settings_get_double (settings, band));
	  g_free (band);
	}
      else
	{
	  gtk_range_set_value (GTK_RANGE (l->data),
			       eq_presets_values[active][i]);
	}
    }
  equalizer_custom = TRUE;
}

GaruEffects *
garu_effects_new (GtkWindow *window)
{
  return g_object_new (GARU_TYPE_EFFECTS,
		       "modal", TRUE,
		       "transient-for", window,
		       "use-header-bar", TRUE, NULL);
}
