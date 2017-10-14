/*
 * mhwkb_md2html.c
 * Converts md files for the Mental Health and Wellness Knowledge Base
 * https://github.com/andy5995/mhwkb
 *
 * Usage: mhwkb_md2html <dir_with_md_files>
 *
 * Copyright 2017 Andy Alt <andy400-dev@yahoo.com>
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

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "html_functions.h"

#define EXIT_NO_ARGS 2
#define EXIT_OPENDIR_FAILURE 4

#define VERSION ".0.0.05"
#define DATE "2017-10-11"

#define MAX_TAG_COUNT 500

void erase_char (char *str, char c);
void trim_char (char *str, char c);

int exists (const char *filename);

int
main (int argc, char **argv)
{
  char *starting_dir = getenv ("PWD");

  if (argc == 1)
  {
    printf ("Usage: %s <md_file_dir>\n", argv[0]);
    printf ("  html files will be output to $PWD: %s\n\n", starting_dir);
    exit (EXIT_NO_ARGS);
  }

  char index_html[256];
  strcpy (index_html, starting_dir);
  strcat (index_html, "/");
  strcat (index_html, "index.html");

  if (exists (index_html) != 0)
  {
    char title_main[256];
    strcpy (title_main, "Home (Under Construction)");
    strcat (title_main, " - Mental Health and Wellness Knowledge Base");
    create_html_header (index_html, title_main);
  }

  struct dirent *entry;
  DIR *files;
  files = opendir (argv[1]);
  if (files == NULL)
  {
    perror ("opendir");
    exit (EXIT_OPENDIR_FAILURE);
  }

  printf ("%s\n", argv[1]);

  int status;
  chdir (argv[1]);
  while ((entry = readdir (files)) != NULL)
  {
    /* needs fixing. Only read .md files */
    if (strcmp (entry->d_name, ".md") == 0)
      continue;

    FILE *md_file = fopen (entry->d_name, "r");

    if (md_file == NULL)
    {
      perror ("failure: open file\n");
      exit (1);
    }

    char link_href[256];
    char link_title[256];
    char md_line[512];

    char tags[20][80];

    int i, href_pos;

    char *tag_Ptr = malloc (sizeof (char *));

    /* accumulates tags used to determine if the rel=nofollow attr
     * should be applied to <a> tags
     * Not yet implemented
     */
    char combined_tags[MAX_TAG_COUNT][256];

    while (fgets (md_line, 512, md_file) != NULL)
    {
      link_href[0] = '\0';
      link_title[0] = '\0';

      if (md_line[0] == '[')
      {
        char tag_html[256];

        FILE *fp_index;
        fp_index = fopen (index_html, "a");

        if (fp_index == NULL)
        {
          perror ("failure: open file\n");
          exit (1);
        }

        for (i = 0; md_line[i] != ']'; i++)
        {
          if (md_line[i] != '[')
          {
            link_title[i - 1] = md_line[i];
          }
        }

        link_title[i - 1] = '\0';

        i++;

        href_pos = 0;

        for (; md_line[i] != ')'; i++)
        {
          if (md_line[i] != '(')
            link_href[href_pos++] = md_line[i];
        }
        link_href[href_pos++] = '\0';
        fprintf (fp_index, "<a class=\"title\" href=\"%s\">%s</a><br />\n", link_href, link_title);

        fgets (md_line, 512, md_file);
        char *date_line = malloc (sizeof (date_line));
        strcpy (date_line, md_line);
        fprintf (fp_index, "<p class=\"date\">date: %s</p>\n", date_line);

        /* get the tags */
        fgets (md_line, 512, md_file);

        tag_Ptr = strtok (md_line, ",");

        int tag_ctr = 0;

        while (tag_Ptr != NULL)
        {
          strcpy (tags[tag_ctr], tag_Ptr);
          /* printf ("%s\n", tags[tag_ctr]); */
          tag_Ptr = strtok (NULL, ",");

          tag_ctr++;
        }

        for (i = 0; i < tag_ctr; i++)
        {
          while (tags[i][0] != '[' && i == 0)
          {
            erase_char (tags[i], tags[i][0]);
          }

          erase_char (tags[i], '[');

          /* if there's any white space between the [ and the " */

          while (tags[i][0] != '"')
          {
            erase_char (tags[i], tags[i][0]);
          }

          erase_char (tags[i], '"');

          /* check to see if we're on the last tag */

          if (i < tag_ctr - 1)
          {
            trim_char (tags[i], '"');
            strcpy (tag_html, tags[i]);
            strcat (tag_html, ".html");
            fprintf (fp_index, "<a href=\"%s\">%s</a>, ", tag_html, tags[i]);
          }
          else
          {
            int pos = 0;

            while (tags[i][pos] != '"')
            {
              pos++;
            }
            /* if were' on the last tag, cut off the "]<br />" */
            tags[i][pos] = '\0';
            strcpy (tag_html, tags[i]);
            strcat (tag_html, ".html");
            fprintf (fp_index, "<a href=\"%s\">%s</a><br /><br />\n", tag_html, tags[i]);
          }
        }

        /* Now that we know all the tags for one entry, make the <tag>.html
         * files
         */
        for (i = 0; i < tag_ctr; i++)
        {
          char html_tag_file[256];
          strcpy (html_tag_file, starting_dir);
          strcat (html_tag_file, "/");
          strcat (html_tag_file, tags[i]);
          strcat (html_tag_file, ".html");

          char title_tag[256];
          strcpy (title_tag, tags[i]);
          strcat (title_tag, " (Under Construction)");
          strcat (title_tag, " - Mental Health and Wellness Knowledge Base");

          if (exists (html_tag_file) != 0)
          {
            create_html_header (html_tag_file, title_tag);
          }

          FILE *fp;

          fp = fopen (html_tag_file, "a");

          if (fp == NULL)
          {
            perror ("failure: open file\n");
            exit (1);
          }

          fprintf (fp, "<a class=\"title\" href=\"%s\">%s</a><br />\n", link_href, link_title);
          fprintf (fp, "<p class=\"date\">date: %s</p>\n", date_line);

          int tag;
          for (tag = 0; tag < tag_ctr - 1; tag++)
          {
            strcpy (tag_html, tags[tag]);
            strcat (tag_html, ".html");
            fprintf (fp, "<a href=\"%s\">%s</a>, ", tag_html, tags[tag]);
          }

          /* add the last hyperlinked tag in the list (no comma printed) */
          fprintf (fp, "<a href=\"%s\">%s</a><br /><br />", tag_html, tags[tag]);

          if (fclose (fp) != 0)
          {
            perror ("failure: close file\n");
            exit (1);
          }

          printf ("\n");

        }

        free (date_line);

        if (fclose (fp_index) != 0)
        {
          perror ("failure: close file\n");
          exit (1);
        }

      }
    }

    free (tag_Ptr);


    if (fclose (md_file) != 0)
    {
      perror ("failure: close file\n");
      exit (1);
    }

  }

  status = closedir (files);
  if (status)
    perror ("closedir");

  /* struct dirent *entry; */

  files = opendir (starting_dir);

  if (files == NULL)
  {
    perror ("opendir");
    exit (EXIT_OPENDIR_FAILURE);
  }

  chdir (starting_dir);

  while ((entry = readdir (files)) != NULL)
  {
    if ((strcmp (entry->d_name, ".") == 0) || (strcmp (entry->d_name, "..") == 0))
      continue;

    FILE *fp = fopen (entry->d_name, "a");
    if (fp == NULL)
    {
      perror ("failure: open file\n");
      exit (1);
    }

    create_html_footer (fp);

    if (fclose (fp) != 0)
    {
      perror ("failure: close file\n");
      exit (1);
    }
  }

  status = closedir (files);
  if (status)
    perror ("closedir");

  return 0;
}

/**
 * Erases characters from the beginning of a string
 * (i.e. shifts the remaining string to the left
 */
void
erase_char (char *str, char c)
{
  int inc = 0;

  while (str[inc] == c)
    inc++;

  if (!inc)
    return;

  int n = strlen (str);
  int i;

  for (i = 0; i < n - inc; i++)
    str[i] = str[i + inc];

  str[n - inc] = '\0';

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

/**
 * int exists (const char *filename);
 * Checks for the existence of *filename. On error, uses perror() to
 * display the reason
 *
 * return: the return value of lstat()
 */
int
exists (const char *filename)
{
  struct stat st;

  return (lstat (filename, &st));
}
