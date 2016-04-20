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
  GtkDialog  parent_instance;

  GSettings *settings;
  GSList    *eq_scales;
};

G_DEFINE_TYPE (GaruEffects, garu_effects, GTK_TYPE_DIALOG);

static gboolean equalizer_custom_selected = FALSE;
static gboolean equalizer_changed_by_combo_box = FALSE;

static void       garu_effects_init_container              (GaruEffects *self);
static GtkWidget *garu_effects_init_box_equalizer          (GaruEffects *self);
static void       garu_effects_equalizer_scale_changed     (GtkRange    *range,
                                                            gchar       *band);
static void       garu_effects_equalizer_combo_box_changed (GtkComboBox *widget,
                                                            GaruEffects *self);
static GtkWidget *garu_effects_init_box_crossfade          (GaruEffects *self);

static void
garu_effects_finalize (GObject *object)
{
  GaruEffects *self = GARU_EFFECTS (object);
  g_print ("Finalizando efectos\n");
  g_slist_free (self->eq_scales);
  g_object_unref (self->settings);
  G_OBJECT_CLASS (garu_effects_parent_class)->finalize (object);
}

static void
garu_effects_class_init (GaruEffectsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = garu_effects_finalize;
}

static void
garu_effects_init (GaruEffects *self)
{
  self->settings = g_object_ref (garu_utils_get_settings ());
  self->eq_scales = NULL;

  gtk_window_set_title (GTK_WINDOW (self), _("Effects"));
  garu_effects_init_container (self);
}

static void
garu_effects_init_container (GaruEffects *self)
{
  GtkWidget *notebook, *area, *label, *content;

  area = gtk_dialog_get_content_area (GTK_DIALOG (self));
  gtk_container_set_border_width (GTK_CONTAINER (area), 0);

  notebook = gtk_notebook_new ();
  gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);
  gtk_box_pack_start (GTK_BOX (area), notebook, TRUE, TRUE, 0);

  label = gtk_label_new (_("Equalizer"));
  content = garu_effects_init_box_equalizer (self);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), content, label);

  label = gtk_label_new (_("Crossfade"));
  content = garu_effects_init_box_crossfade (self);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), content, label);

  gtk_widget_show_all (area);
}

static GtkWidget *
garu_effects_init_box_equalizer (GaruEffects *self)
{
  gint       i;
  gchar     *text;
  GtkWidget *vbox, *hbox, *grid, *label, *toogle, *combo_box, *scale;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);

  /* Label enabled */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
  label = gtk_label_new (NULL);
  text = garu_utils_text_bold (_("Enabled"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  toogle = gtk_switch_new ();
  g_settings_bind (self->settings, "equalizer-enabled",
                   toogle, "active", G_SETTINGS_BIND_DEFAULT);
  gtk_box_pack_start (GTK_BOX (hbox), toogle, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  /* ComboBox Presets */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 20);
  label = gtk_label_new (_("Presets"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  combo_box = gtk_combo_box_text_new ();
  for (i = 0 ; i < G_N_ELEMENTS (eq_presets); i++)
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo_box),
                                    eq_presets[i]);
  gtk_box_pack_start (GTK_BOX (hbox), combo_box, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  g_settings_bind (self->settings, "equalizer-enabled",
                   hbox, "sensitive", G_SETTINGS_BIND_GET);

  /* Grid equalizer */
  grid = gtk_grid_new ();
  for (i = 0; i < G_N_ELEMENTS (eq_bands); i++)
    {
      text = g_strdup_printf ("eq-custom-band%d", i);
      scale = gtk_scale_new_with_range (GTK_ORIENTATION_VERTICAL,
                                        -12, 12, 0.1);
      label = gtk_label_new (eq_bands [i]);
      gtk_range_set_inverted (GTK_RANGE (scale), TRUE);
      gtk_scale_add_mark (GTK_SCALE (scale),  12.0, GTK_POS_LEFT, NULL);
      gtk_scale_add_mark (GTK_SCALE (scale),   0.0, GTK_POS_LEFT, NULL);
      gtk_scale_add_mark (GTK_SCALE (scale), -12.0, GTK_POS_LEFT, NULL);
      gtk_widget_set_size_request (scale, -1, 200);
      gtk_grid_attach (GTK_GRID (grid), scale, i, 0, 1, 1);
      gtk_grid_attach (GTK_GRID (grid), label, i, 1, 1, 1);
      g_signal_connect (scale, "value-changed",
                        G_CALLBACK (garu_effects_equalizer_scale_changed),
                        text);
      self->eq_scales = g_slist_append (self->eq_scales, scale);
    }
  gtk_box_pack_start (GTK_BOX (vbox), grid, FALSE, FALSE, 0);
  g_settings_bind (self->settings, "equalizer-enabled",
                   grid, "sensitive", G_SETTINGS_BIND_GET);
  g_signal_connect (combo_box, "changed",
                    G_CALLBACK (garu_effects_equalizer_combo_box_changed),
                    self);
  g_settings_bind (self->settings, "equalizer-preset",
                   combo_box, "active", G_SETTINGS_BIND_GET);
  return vbox;
}

static void
garu_effects_equalizer_combo_box_changed (GtkComboBox *widget,
                                          GaruEffects *self)
{
  GSList *l;
  gchar  *band;
  int     active, i;
  active = gtk_combo_box_get_active (widget);
  g_settings_set_int (self->settings,"equalizer-preset", active);
  equalizer_custom_selected = (active == G_N_ELEMENTS (eq_presets) - 1);
  equalizer_changed_by_combo_box = TRUE;
  for (l = self->eq_scales; l != NULL; l = l->next)
    {
      i = g_slist_position (self->eq_scales, l);
      /* If custom preset is activated */
      if (equalizer_custom_selected)
        {
          band = g_strdup_printf ("eq-custom-band%d", i);
          gtk_range_set_value (GTK_RANGE (l->data),
                               g_settings_get_double (self->settings, band));
          g_free (band);
        }
      else
        {
          gtk_range_set_value (GTK_RANGE (l->data),
                               eq_presets_values[active][i]);
        }
    }
  equalizer_changed_by_combo_box = FALSE;
}

static void
garu_effects_equalizer_scale_changed (GtkRange  *range,
                                      gchar     *band)
{
  GaruApplication *app;
  GaruPlayer      *player;
  GSettings       *settings;
  gint             active, i;
  gchar           *settings_band;

  app = GARU_APPLICATION (g_application_get_default ());
  settings = garu_application_get_settings (app);
  player = garu_application_get_player (app);

  active = g_settings_get_int (settings, "equalizer-preset");

  if (!equalizer_changed_by_combo_box)
    {
      if (!equalizer_custom_selected)
        {
          for (i = 0; i < G_N_ELEMENTS (eq_bands); i++)
            {
              settings_band = g_strdup_printf ("eq-custom-band%d", i);
              g_settings_set_double (settings, settings_band,
                                     eq_presets_values[active][i]);
              g_free (settings_band);
            }
          g_settings_set_int (settings, "equalizer-preset",
                              G_N_ELEMENTS (eq_presets) - 1);
          equalizer_custom_selected = TRUE;
        }
      g_settings_set_double (settings, band, gtk_range_get_value (range));
    }
  garu_player_update_equalizer (player);
}

static GtkWidget *
garu_effects_init_box_crossfade (GaruEffects *self)
{
  gchar     *text;
  GtkWidget *vbox, *hbox, *label, *toogle, *check_button, *spin_button;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);

  /* Label enabled */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  label = gtk_label_new (NULL);
  text = garu_utils_text_bold (_("Enabled"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  toogle = gtk_switch_new ();
  g_settings_bind (self->settings, "crossfade-enabled",
                   toogle, "active", G_SETTINGS_BIND_DEFAULT);
  gtk_box_pack_start (GTK_BOX (hbox), toogle, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  /* Spin button */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
  label = gtk_label_new (_("Crossfade duration in seconds"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  spin_button = gtk_spin_button_new_with_range (1, 12, 1.0);
  g_settings_bind (self->settings, "crossfade-time",
                   spin_button, "value", G_SETTINGS_BIND_DEFAULT);
  gtk_box_pack_start (GTK_BOX (hbox), spin_button, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  g_settings_bind (self->settings, "crossfade-enabled",
                   hbox, "sensitive", G_SETTINGS_BIND_GET);

  /* Check buttons options */
  text = g_strdup (_("Crossfade when the user change the track"));
  check_button = gtk_check_button_new_with_label (text);
  g_free (text);
  gtk_box_pack_start (GTK_BOX (vbox), check_button, FALSE, FALSE, 0);
  g_settings_bind (self->settings, "crossfade-change-track",
                   check_button, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (self->settings, "crossfade-enabled",
                   check_button, "sensitive", G_SETTINGS_BIND_GET);

  text = g_strdup (_("Crossfade before the track ends"));
  check_button = gtk_check_button_new_with_label (text);
  g_free (text);
  gtk_box_pack_start (GTK_BOX (vbox), check_button, FALSE, FALSE, 0);
  g_settings_bind (self->settings, "crossfade-track-ends",
                   check_button, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (self->settings, "crossfade-enabled",
                   check_button, "sensitive", G_SETTINGS_BIND_GET);

  return vbox;
}

GaruEffects *
garu_effects_new (GtkWindow *window)
{
  return g_object_new (GARU_TYPE_EFFECTS,
                       "modal", TRUE,
                       "transient-for", window,
                       "use-header-bar", TRUE, NULL);
}
