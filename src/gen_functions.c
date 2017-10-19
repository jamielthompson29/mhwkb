/*
 * gen_functions.c
 * Converts md files for the Mental Health and Wellness Knowledge Base
 * https://github.com/andy5995/mhwkb
 *
 * Copyright 2017 Andy Alt <andy400-dev@yahoo.com>
 * With contributions from
 *
 * Daniel Kelly <myself@danielkelly.me>
 *
 * and others mentioned in
 * https://github.com/andy5995/mhwkb/blob/master/CONTRIBUTING.md
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

#include "gen_functions.h"

/**
 * Erases characters from the beginning of a string
 * (i.e. shifts the remaining string to the left
 */
void
del_char_shift_left (char *str, char c)
{
  int c_count = 0;

  /* count how many instances of 'c' */
  while (str[c_count] == c)
    c_count++;

  /* if no instances of 'c' were found... */
  if (!c_count)
    return;

  int len = strlen (str);
  int pos;

  for (pos = 0; pos < len - c_count; pos++)
    str[pos] = str[pos + c_count];

  str[len - c_count] = '\0';

  return;
}

/**
 * Trim a trailing character if present
 */
void
trim_char (char *str, char c)
{
  int len;
  len = strlen(str) - 1;

  if (str[len] != c)
    return;

  str[len] = '\0';

  return;
}

void buf_check (const char *str, const int len)
{
  if (strlen (str) >= len)
  {
    printf ("error: Buffer overflow caught\n");
    printf ("string length: %lu\n", strlen (str));

    int pos = 0;
    int chars_to_print = 0;
    chars_to_print = (len >= 80) ? 80 : 10;

    for (pos = 0; pos < chars_to_print; pos++)
      printf ("%c", str[pos]);

    printf ("\n");

    exit (1);
  }

  return;
}

/* tokenize the "tags" line */
int parse_tags_line (char *line, char tags[][TAG_MAX_LEN])
{
  char *tag_Ptr;
  tag_Ptr = strtok (line, ",");

  /* pointer to each tag. Used to check for a space in a tag and
   * replace to an underscore */
  char *tp;
  int local_tag_ctr = 0;

  while (tag_Ptr != NULL)
  {

    tp = &tag_Ptr[0];

    while (*tp != '\0')
    {
      if (*tp != ' ')
      {}
      else
        *tp = '_';

      tp++;
    }

    strcpy (tags[local_tag_ctr], tag_Ptr);

    tag_Ptr = strtok (NULL, ",");

    local_tag_ctr++;
  }

  return local_tag_ctr;
}
