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

#ifndef SRC_COLORS_H_
#define SRC_COLORS_H_

typedef struct _Color {
    char *colorName;
    char *colorValue;
} Color;

extern Color *colors;

#endif /* SRC_COLORS_H_ */
