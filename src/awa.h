/*
 *
 *  EW17 Bulb Controller
 *
 *  Copyright (C) 2017  Imagination Technologies Limited
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef SRC_AWA_H_
#define SRC_AWA_H_

#include <stdbool.h>
#include <glib.h>
#include <awa/client.h>

#define OPERATION_PERFORM_TIMEOUT 1000
#define LIGHT_CONTROL_OBJECT_ID 3311
#define COLOUR_RESOURCE_ID 5706

typedef void (*ColorChangedCB)(const char* new_color);

AwaClientSession* awa_setup(ColorChangedCB, GError**);

void awa_cleanup(void);

gboolean awa_process(gpointer user_data);

void awa_get_current_color(char **color);

#endif /* SRC_AWA_H_ */
