/*
 * mhwkb_md2html.c
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

#include "main.h"

int
main (int argc, char **argv)
{
  /* FIXME: Check for existence of directories */
  if (argc != 3)
  {
    printf ("Usage: %s <md_file_dir> <starting_dir>\n", argv[0]);
    exit (EXIT_INVALID_ARGS);
  }

  /* FIXME: This var should be changed to "output_dir" */
  char starting_dir[PATH_MAX];

  if (realpath (argv[2], starting_dir) == NULL)
  {
    perror ("realpath: failure");
    exit (EXIT_FAILURE);
  }

  strcat (starting_dir, "/");
  /* FIXME: display this and ask for confirmation */
  printf ("  html files will be output to: %s\n\n", starting_dir);

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
      printf ("%s\n", entry->d_name);
      exit (1);
    }

    char link_href[LINK_MAX_LEN];
    char link_title[256];
    char md_line[LINE_MAX_LEN];

    char tags[TAG_MAX_NUM][TAG_MAX_LEN];

    int i, href_pos;

    /* Pagination:
     * First pass through the while loop only count the number of links,
     * and get the number of links that will appear in each <tag.html> file
     *
     * Not yet implemented
     */
    int pass;
    for (pass = 1; pass <= PASSES; pass++)
    {
      while (fgets (md_line, LINE_MAX_LEN, md_file) != NULL)
      {
        link_href[0] = '\0';
        link_title[0] = '\0';

        /* check to see if there's a link on that line (GitHub markdown) */
        if (md_line[0] == '[')
        {
          char tag_html[HTML_FILENAME_MAX_LEN];

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
          fgets (md_line, LINE_MAX_LEN, md_file);
          char *date_line = malloc (128);
          memset(date_line, 0, 128);
          strcpy (date_line, md_line);

          /* get the tags */
          fgets (md_line, LINE_MAX_LEN, md_file);

          int tag_ctr = parse_tags_line (md_line, tags);

          char *article_links = malloc(TAGS_COMBINED_MAX_LEN + 1);
          memset(article_links, 0, TAGS_COMBINED_MAX_LEN + 1);

          for (i = 0; i < tag_ctr; i++)
          {
            while (tags[i][0] != '[' && i == 0)
            {
              del_char_shift_left (tags[i], tags[i][0]);
            }

            del_char_shift_left (tags[i], '[');

            /* if there's any white space between the [ and the " */

            while (tags[i][0] != '"')
            {
              del_char_shift_left (tags[i], tags[i][0]);
            }

            del_char_shift_left (tags[i], '"');

            /* check to see if we're on the last tag */

            if (i < tag_ctr - 1)
            {
              trim_char (tags[i], '"');

              sprintf (tag_html, "%s.html", tags[i]);
            }
            else
            {
              int pos = 0;

              while (tags[i][pos] != '"')
              {
                pos++;
              }
              /* if we're on the last tag, cut off the "]<br />" */
              tags[i][pos] = '\0';
              sprintf (tag_html, "%s.html", tags[i]);
            }

            const char *keys[] = { "link", "title" };

            buf_check (tags[i], TAG_MAX_LEN);

            const char *values[] = { tag_html, tags[i] };
            char *article_link = render_template_file(TEMPLATE_ARTLNK_PATH, 2, keys, values);

            buf_check (article_link, LINK_MAX_LEN);

            strcat(article_links, article_link);
            free(article_link);

            strcat(article_links, "\n");
            buf_check (article_links, TAGS_COMBINED_MAX_LEN);
          }

          // Render the article templates
          const char *keys[] = { "link", "title", "date", "article_links" };
          const char *values[] = { link_href, link_title, date_line, article_links };
          char *article_template = render_template_file(TEMPLATE_ARTICLE_PATH, 4, keys, values);
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
            char html_tag_file[HTML_FILENAME_MAX_LEN];
            sprintf (html_tag_file, "%s%s.html", starting_dir, tags[i]);

            char title_tag[256];

            strcpy (title_tag, tags[i]);
            strcat (title_tag, " (Under Construction)");
            strcat (title_tag, " - Mental Health and Wellness Knowledge Base");

            char tags_tag[TAGS_COMBINED_MAX_LEN + 1];
            memset(tags_tag, 0, TAGS_COMBINED_MAX_LEN + 1);
            int tag;
            for (tag = 0; tag < tag_ctr; tag++)
            {
              strcpy (tag_html, tags[tag]);
              strcat (tag_html, ".html");

              const char *link_keys[] = { "link", "title" };
              const char *link_values[] = { tag_html, tags[tag] };
              char *link_template = render_template_file(TEMPLATE_ARTLNK_PATH, 2, link_keys, link_values);

              strcat (tags_tag, link_template);

              free(link_template);
            }

            // Render the article templates
            const char *article_keys[] = { "link", "title", "date", "article_links" };
            const char *article_values[] = { link_href, link_title, date_line, tags_tag };
            char *article_template = render_template_file(TEMPLATE_ARTICLE_PATH, 4, article_keys, article_values);

            // Save the file
            FILE *fp = fopen (html_tag_file, "a");
            if (fp == NULL)
            {
              perror ("failure: open file\n");
              printf ("%s\n", html_tag_file);
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
    }

    if (fclose (md_file) != 0)
    {
      perror ("failure: close file\n");
      exit (1);
    }
  }
  free(files);

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
    char *index_template = render_template_file (TEMPLATE_INDEX_PATH, 2, index_keys, index_values);
    free(tag_contents);

    FILE *fp = fopen (entry->d_name, "w+");
    if (fp == NULL)
    {
      perror ("failure: open file\n");
      printf ("%s\n", entry->d_name);
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
  free(article_files);

  // Write the index file
  char index_html[256];
  sprintf (index_html, "%s/index.html", starting_dir);

  char title_main[256];
  title_main[0] = '\0';
  if (exists (index_html) != 0)
  {
    strcpy (title_main, "Home");
    strcat (title_main, " - Mental Health and Wellness Knowledge Base");
  }

  FILE *fp_index = fopen (index_html, "a");
  if (fp_index == NULL)
  {
    perror ("failure: open file\n");
    printf ("%s\n", index_html);
    exit (1);
  }

  int all_content_length = 0;
  int i = 0;
  for(i = 0; i < article_length; ++i)
  {
    all_content_length += strlen(articles[i]);
  }
  char all_articles[all_content_length + 1];
  memset(all_articles, 0, all_content_length + 1);
  for(i = 0; i < article_length; ++i)
  {
    strcat(all_articles, articles[i]);
    free(articles[i]);
  }

  const char *index_keys[] = { "title", "body" };
  const char *index_values[] = { title_main, all_articles };
  char *index_template = render_template_file(TEMPLATE_INDEX_PATH, 2, index_keys, index_values);
  fprintf (fp_index, "%s", index_template);
  free(index_template);

  if (fclose (fp_index) != 0)
  {
    perror ("failure: close file\n");
    exit (1);
  }

  return 0;
}
