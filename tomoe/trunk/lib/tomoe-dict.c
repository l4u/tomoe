/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2000 - 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 *  Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tomoe-dict.h"

#define LINE_BUF_SIZE 4096
#define DICT_LETTER_INITIAL_SIZE 3049
#define DICT_LETTER_EXPAND_SIZE 10

struct _tomoe_dict
{
    char         *file_name;
    char         *dict_name;
    char         *encoding;
    char         *lang;
    int           letter_num;
    tomoe_letter *letters;
};

tomoe_dict *
tomoe_dict_new (const char *filename)
{
    tomoe_dict *dict;
    char *p = NULL;
    int letter_num = 0;
    int stroke_num = 0;
    int point_num = 0;
    int i = 0, j = 0, k = 0;
    tomoe_letter *lttr;
    tomoe_stroke *strk = NULL;
    tomoe_point  *pnt  = NULL;
    char line_buf[LINE_BUF_SIZE];
    FILE *fp;

    if (!filename && !*filename) return NULL;

    fp = fopen (filename, "r");
    if (!fp) return NULL;

    dict = calloc (1, sizeof (tomoe_dict));
    dict->letter_num = DICT_LETTER_INITIAL_SIZE;
    dict->letters    = calloc (dict->letter_num, sizeof (tomoe_letter));

    while ((p = fgets (line_buf, LINE_BUF_SIZE, fp)) != NULL)
    {
        if (p[0] == '\n')
            continue;

        ++letter_num;
        if (letter_num > dict->letter_num)
        {
            dict->letter_num += DICT_LETTER_EXPAND_SIZE;
            dict->letters = realloc (dict->letters,
                                     sizeof (tomoe_letter) * dict->letter_num);
        }

        i = letter_num - 1;
        lttr = &dict->letters[i];
        p = strchr (p, '\n');
        if (p != NULL)
            *p = '\0';
        lttr->character = strdup (line_buf);

        p = fgets (line_buf, LINE_BUF_SIZE, fp);
        if (p == NULL)
            break;
        if (p[0] != ':')
            continue;

        sscanf (p + 1, "%d", &stroke_num);

        lttr->c_glyph = calloc (1, sizeof (tomoe_glyph));
        tomoe_glyph_init_with_strokes (lttr->c_glyph, stroke_num);

        for (j = 0; j < stroke_num; j++)
        {
            strk = &lttr->c_glyph->strokes[j];
            p = fgets (line_buf, LINE_BUF_SIZE, fp);
            sscanf (p, "%d", &point_num);
            p = strchr (p, ' ');
            tomoe_stroke_init_with_points (strk, point_num);
            for (k = 0; k < point_num; k++)
            {
                pnt = &strk->points[k];
                sscanf (p, " (%d %d)", &pnt->x, &pnt->y);
                p = strchr (p, ')') + 1;
            }

            /*stroke_calculate_metrics (strk);*/
        }
    }
    fclose (fp);

    if (letter_num < dict->letter_num)
        dict->letter_num = letter_num;

    return dict;
}

void
tomoe_dict_free (tomoe_dict *dict)
{
    int i;

    if (!dict) return;

    if (dict->letters != NULL)
    {
        for (i = 0; i < dict->letter_num; i++)
            tomoe_letter_clear (&dict->letters[i]);
        free (dict->letters);
        dict->letters = NULL;
    }

    free (dict);
}

const char *
tomoe_dict_get_file_name (tomoe_dict *dict)
{
    if (!dict) return NULL;
    return dict->file_name;
}

const char *
tomoe_dict_get_name (tomoe_dict *dict)
{
    if (!dict) return NULL;
    return dict->dict_name;
}

unsigned int
tomoe_dict_get_number_of_letters (tomoe_dict *dict)
{
    if (!dict) return 0;
    return dict->letter_num;
}

const tomoe_letter *
tomoe_dict_get_letters (tomoe_dict *dict)
{
    if (!dict) return NULL;
    return dict->letters;
}
