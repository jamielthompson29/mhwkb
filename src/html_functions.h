/*
 * html_functions.h
 *
 * Copyright 2017 Andy <andy@oceanus>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#ifndef HTML_FUNCTIONS_H
#define HTML_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>

/* function prototypes */
void create_html_header (char *html_file, char *title);
void create_html_footer (FILE *html_file);

#endif
