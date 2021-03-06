/* garu-widgets.h
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

#ifndef GARU_WIDGETS_H
#define GARU_WIDGETS_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GARU_TYPE_WIDGETS (garu_widgets_get_type())

G_DECLARE_DERIVABLE_TYPE (GaruWidgets, garu_widgets, GARU, WIDGETS, GObject)

struct _GaruWidgetsClass
{
	GObjectClass parent;
};

GaruWidgets *garu_widgets_new (void);

G_END_DECLS

#endif /* GARU_WIDGETS_H */
