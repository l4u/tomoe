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

#if 1
/*
 * FIXME! Move to other file.
 */
static void
tomoe_stroke_new (tomoe_stroke *strk, int point_num)
{
    strk->point_num = point_num;
    strk->points    = calloc (point_num, sizeof (tomoe_point));
}

static void
tomoe_stroke_free (tomoe_stroke *strk)
{
    if (strk)
        return;

    if (strk->points != NULL)
    {
        free (strk->points);
        strk->points = NULL;
    }
}

static void
tomoe_letter_new (tomoe_letter *lttr, int stroke_num)
{
    lttr->c_glyph             = calloc (1, sizeof (tomoe_glyph));
    lttr->c_glyph->stroke_num = stroke_num;
    lttr->c_glyph->strokes    = calloc (stroke_num, sizeof (tomoe_stroke));
}

static void
tomoe_letter_free (tomoe_letter *lttr)
{
    int i;

    if (!lttr) return;

    if (lttr->character != NULL)
    {
        free (lttr->character);
        lttr->character = NULL;
    }

    if (lttr->c_glyph != NULL)
    {
        for (i = 0; i < lttr->c_glyph->stroke_num; i++)
        {
            tomoe_stroke_free (&lttr->c_glyph->strokes[i]);
        }
        free (lttr->c_glyph->strokes);
        free (lttr->c_glyph);
        lttr->c_glyph->strokes = NULL;
        lttr->c_glyph          = NULL;
    }
}

void
tomoe_glyph_free (tomoe_glyph *glyph)
{
    int i;

    if (!glyph) return;

    for (i = 0; i < glyph->stroke_num; i++)
    {
        tomoe_stroke_free (&glyph->strokes[i]);
    }
    if (glyph->strokes) {
        free (glyph->strokes);
        glyph->strokes = NULL;
    }

    free (glyph);
}
#endif

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

    if (!filename && !*filename)
        return NULL;

    fp = fopen (filename, "r");
    if (!fp)
        return NULL;

    dict = calloc (1, sizeof (tomoe_dict));
    dict->letter_num = DICT_LETTER_INITIAL_SIZE;
    dict->letters    = calloc (letter_num,
                                 sizeof (DICT_LETTER_INITIAL_SIZE));

    while ((p = fgets (line_buf, LINE_BUF_SIZE, fp)) != NULL)
    {
        if (p[0] == '\n')
        {
            continue;
        }
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
        {
            *p = '\0';
        }
        lttr->character = strdup (line_buf);

        p = fgets (line_buf, LINE_BUF_SIZE, fp);
        if (p == NULL)
        {
            break;
        }
        if (p[0] != ':')
        {
            continue;
        }

        sscanf (p + 1, "%d", &stroke_num);

        tomoe_letter_new (lttr, stroke_num);

        for (j = 0; j < stroke_num; j++)
        {
            strk = &lttr->c_glyph->strokes[j];
            p = fgets (line_buf, LINE_BUF_SIZE, fp);
            sscanf (p, "%d", &point_num);
            p = strchr (p, ' ');
            tomoe_stroke_new (strk, point_num);
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
    {
        dict->letter_num = letter_num;
    }

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
        {
            tomoe_letter_free (&dict->letters[i]);
        }
        free (dict->letters);
        dict->letters = NULL;
    }

    free (dict);
}

const char *
tomoe_dict_get_file_name (tomoe_dict *dict)
{
    if (!dict)
        return NULL;
    return dict->file_name;
}

const char *
tomoe_dict_get_name (tomoe_dict *dict)
{
    if (!dict)
        return NULL;
    return dict->dict_name;
}

unsigned int
tomoe_dict_get_number_of_letters (tomoe_dict *dict)
{
    if (!dict)
        return 0;
    return dict->letter_num;
}

const tomoe_letter *
tomoe_dict_get_letters (tomoe_dict *dict)
{
    if (!dict)
        return NULL;
    return dict->letters;
}
