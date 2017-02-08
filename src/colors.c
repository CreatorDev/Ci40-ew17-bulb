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

#include "colors.h"

static Color colortmp[] = {
        {
            .colorName="red",
            .colorValue="56FF000000F0AA"
        },
        {
            .colorName="blue",
            .colorValue="560000FF00F0AA"
        },
        {
            .colorName="green",
            .colorValue="5600FF0000F0AA"

        },
        {
            .colorName="yellow",
            .colorValue ="56FFFF0000F0AA"
        },
        {
            .colorName="none",
            .colorValue="5600000000F0AA"
        }
};

Color *colors = colortmp;
