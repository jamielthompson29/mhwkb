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
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "template_functions.h"

#define EXIT_NO_ARGS 2
#define EXIT_OPENDIR_FAILURE 4

#define VERSION ".0.0.05"
#define DATE "2017-10-11"

#define MAX_ARTICLES 500
#define MAX_TAG_COUNT 500

void erase_char (char *str, char c);
void trim_char (char *str, char c);

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

  struct dirent *entry;
  DIR *files;
  files = opendir (argv[1]);
  if (files == NULL)
  {
    perror ("opendir");
    exit (EXIT_OPENDIR_FAILURE);
  }

  printf ("%s\n", argv[1]);

  char *articles[MAX_ARTICLES];
  int article_length = 0;

  int status;
  chdir (argv[1]);
  while ((entry = readdir (files)) != NULL)
  {
    /* Only read .md files */
    if (strstr (entry->d_name, ".md") == NULL)
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

        // Get the date
        fgets (md_line, 512, md_file);
        char *date_line = malloc (sizeof (date_line));
        strcpy (date_line, md_line);

        /* get the tags */
        fgets (md_line, 512, md_file);

        /* tokenize the "tags" line */

        tag_Ptr = strtok (md_line, ",");

        int tag_ctr = 0;

        /* pointer to each tag. Used to check for a space in a tag and
         * replace to an underscore */
        char *tp;

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

          strcpy (tags[tag_ctr], tag_Ptr);

          tag_Ptr = strtok (NULL, ",");

          tag_ctr++;
        }

        char *article_links = malloc(512 + 1);
        memset(article_links, 0, 512 + 1);

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
          }

          const char *keys[] = { "link", "title" };
          const char *values[] = { tag_html, tags[i] };
          char *article_link = render_template("../templates/article_link.html", 2, keys, values);
          strcat(article_links, article_link);
          free(article_link);

          if (i < tag_ctr - 1)
            strcat(article_links, ", ");
          else
            strcat(article_links, "<br /><br />\n");
        }

        // Render the article templates
        const char *keys[] = { "link", "title", "date", "article_links" };
        const char *values[] = { link_href, link_title, date_line, article_links };
        char *article_template = render_template("../templates/article.html", 4, keys, values);
        free(article_links);

        // Error if we are about to over flow MAX_ARTICLES
        if(article_length >= MAX_ARTICLES) {
          perror ("failure: over MAX_ARTICLES");
          exit(1);
        }

        articles[article_length++] = article_template;

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

          char tags_tag[512+1];
          memset(tags_tag, 0, 512 + 1);
          int tag;
          for (tag = 0; tag < tag_ctr - 1; tag++)
          {
            strcpy (tag_html, tags[tag]);
            strcat (tag_html, ".html");

            const char *link_keys[] = { "link", "title" };
            const char *link_values[] = { tag_html, tags[tag] };
            char *link_template = render_template("../templates/article_link.html", 2, link_keys, link_values);

            strcat (tags_tag, link_template);
            strcat (tags_tag, ", ");

            free(link_template);
          }

          const char *link_keys[] = { "link", "title" };
          const char *link_values[] = { tag_html, tags[tag] };
          char *link_template = render_template("../templates/article_link.html", 2, link_keys, link_values);

          strcat (tags_tag, link_template);
          strcat (tags_tag, "<br /><br />");

          free(link_template);

          // Render the article templates
          const char *article_keys[] = { "link", "title", "date", "article_links" };
          const char *article_values[] = { link_href, link_title, date_line, tags_tag };
          char *article_template = render_template("../templates/article.html", 4, article_keys, article_values);

          // Save the file
          FILE *fp = fopen (html_tag_file, "a");
          if (fp == NULL)
          {
            perror ("failure: open file\n");
            exit (1);
          }

          fprintf (fp, "%s", article_template);
          free(article_template);

          if (fclose (fp) != 0)
          {
            perror ("failure: close file\n");
            exit (1);
          }
        }

        free (date_line);
      }
    }

    free (tag_Ptr);

    if (fclose (md_file) != 0)
    {
      perror ("failure: close file\n");
      exit (1);
    }
  }

  // Read the directory for all articles that are not index
  // Overwrite with the contents inserted into the index template
  // for the header and footer
  DIR *article_files = opendir (starting_dir);
  if (article_files == NULL)
  {
    perror ("opendir");
    exit (EXIT_OPENDIR_FAILURE);
  }

  chdir (starting_dir);
  while ((entry = readdir (article_files)) != NULL)
  {
    // Get the file contents to insert inside the template and rewrite
    if (strstr (entry->d_name, ".html") == NULL)
      continue;

    int title_len = strlen(entry->d_name) - 5;
    char title_tag[title_len + 1];
    memset(title_tag, 0, title_len + 1);
    strncpy(title_tag, entry->d_name, title_len);

    char fullfilename[strlen(starting_dir) + 1 + strlen(entry->d_name) + 1];
    memset(fullfilename, 0, strlen(starting_dir) + 1 + strlen(entry->d_name) + 1);
    strcat(fullfilename, starting_dir);
    strcat(fullfilename, "/");
    strcat(fullfilename, entry->d_name);

    printf("%s\n", fullfilename);

    char* tag_contents = read_file_contents(entry->d_name);
    if(tag_contents == NULL)
    {
      perror("failure: file not read");
      exit(1);
    }

    // Render the index templates
    const char *index_keys[] = { "title", "body" };
    const char *index_values[] = { title_tag, tag_contents };
    char *index_template = render_template ("./templates/index.html", 2, index_keys, index_values);
    free(tag_contents);

    FILE *fp = fopen (entry->d_name, "w+");
    if (fp == NULL)
    {
      perror ("failure: open file\n");
      exit (1);
    }

    fprintf (fp, "%s", index_template);
    free(index_template);

    if (fclose (fp) != 0)
    {
      perror ("failure: close file\n");
      exit (1);
    }
  }

  // Write the index file
  char index_html[256];
  strcpy (index_html, starting_dir);
  strcat (index_html, "/");
  strcat (index_html, "index.html");

  char title_main[256];
  title_main[0] = '\0';
  if (exists (index_html) != 0)
  {
    strcpy (title_main, "Home (Under Construction)");
    strcat (title_main, " - Mental Health and Wellness Knowledge Base");
  }

  FILE *fp_index = fopen (index_html, "a");
  if (fp_index == NULL)
  {
    perror ("failure: open file\n");
    exit (1);
  }

  int all_content_length = 0;
  for(int x = 0; x < article_length; ++x)
  {
    all_content_length += strlen(articles[x]);
  }
  char all_articles[all_content_length + 1];
  memset(all_articles, 0, all_content_length + 1);
  for(int x = 0; x < article_length; ++x)
  {
    strcat(all_articles, articles[x]);
    free(articles[x]);
  }

  const char *index_keys[] = { "title", "body" };
  const char *index_values[] = { title_main, all_articles };
  char *index_template = render_template("./templates/index.html", 2, index_keys, index_values);
  fprintf (fp_index, "%s", index_template);
  free(index_template);

  if (fclose (fp_index) != 0)
  {
    perror ("failure: close file\n");
    exit (1);
  }

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
