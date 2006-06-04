/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2000 - 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
 *  Copyright (C) 2006 Juernjakob Harder <juernjakob.harder@gmail.com>
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
#include <libxml/xmlreader.h>

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
    unsigned int  letter_num;
    tomoe_letter *letters;
    unsigned int  ref_count;
};

tomoe_dict *
tomoe_dict_new (const char *filename)
{
    tomoe_dict *dict;
    char *p = NULL;
    unsigned int letter_num = 0;
    int stroke_num = 0;
    int point_num = 0;
    int i = 0, j = 0, k = 0;
    tomoe_letter *lttr;
    tomoe_stroke *strk = NULL;
    tomoe_point  *pnt  = NULL;
    char line_buf[LINE_BUF_SIZE];
    FILE *fp;
    xmlTextReaderPtr reader;
    int res;
    int parse_mode = 0;

    LIBXML_TEST_VERSION

    if (!filename && !*filename) return NULL;

    reader = xmlReaderForFile(filename, NULL, 0);
    if (!reader) return NULL;

    dict = calloc (1, sizeof (tomoe_dict));
    dict->letter_num = DICT_LETTER_INITIAL_SIZE;
    dict->letters    = calloc (dict->letter_num, sizeof (tomoe_letter));

    // FIX-ME: clean up, error handling

    res = xmlTextReaderRead(reader);
    while (res == 1)
    {
        const xmlChar *name, *value;

        name = xmlTextReaderConstName(reader);
        if (name == NULL)
            name = BAD_CAST "--";

        switch (parse_mode)
        {
        case 0:
            if (0 == xmlStrcmp(name, BAD_CAST "dict"))
                parse_mode = 1;
            break;
        case 1:
            if (0 == xmlStrcmp(name, BAD_CAST "character"))
            {
                parse_mode = 2;

                ++letter_num;
                if (letter_num > dict->letter_num)
                {
                    dict->letter_num += DICT_LETTER_EXPAND_SIZE;
                    dict->letters = realloc (dict->letters,
                        sizeof (tomoe_letter) * dict->letter_num);
                }

                i = letter_num - 1;
                lttr = &dict->letters[i];
            }
            else if (0 == xmlStrcmp(name, BAD_CAST "dict") && 
                     15 == xmlTextReaderNodeType(reader))
                parse_mode = 0;
            break;
        case 2:
            if (0 == xmlStrcmp(name, BAD_CAST "literal"))
                parse_mode = 3;
            else if (0 == xmlStrcmp(name, BAD_CAST "strokelist"))
            {
                sscanf(xmlTextReaderGetAttribute(reader, BAD_CAST "count"), "%d", &stroke_num);
                lttr->c_glyph = calloc(1, sizeof (tomoe_glyph));
                tomoe_glyph_init(lttr->c_glyph, stroke_num);
                j = 0;
                parse_mode = 4;
            }
            else if (1 == xmlTextReaderDepth(reader))
                parse_mode = 1;
            break;
        case 3:
            if (0 == xmlStrcmp(name, BAD_CAST "literal") && 
                15 == xmlTextReaderNodeType(reader))
                parse_mode = 2;
            else
            {
                lttr->character = strdup (xmlTextReaderConstValue(reader));
            }
            break;
        case 4:
            if (0 == xmlStrcmp(name, BAD_CAST "s"))
            {
                sscanf (xmlTextReaderGetAttribute(reader, BAD_CAST "count"), "%d", &point_num);
                parse_mode = 5;
            }
            else if (0 == xmlStrcmp(name, BAD_CAST "strokelist") && 
                     15 == xmlTextReaderNodeType(reader))
                parse_mode = 2;
            break;
        case 5:
            if (0 == xmlStrcmp(name, BAD_CAST "s") && 
                15 == xmlTextReaderNodeType(reader))
            {
                parse_mode = 4;
                j++;
            }
            else
            {
                strk = &lttr->c_glyph->strokes[j];
                tomoe_stroke_init (strk, point_num);
                p = BAD_CAST xmlTextReaderConstValue(reader);
                for (k = 0; k < point_num; k++)
                {
                    pnt = &strk->points[k];
                    sscanf (p, " (%d %d)", &pnt->x, &pnt->y);
                    p = strchr (p, ')') + 1;
                }
            }
            break;
        default:;
        }

        res = xmlTextReaderRead(reader);
    }

    xmlFreeTextReader(reader);

    if (letter_num < dict->letter_num)
        dict->letter_num = letter_num;

    dict->ref_count = 1;

    return dict;
}

void
tomoe_dict_free (tomoe_dict *dict)
{
    unsigned int i;

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

tomoe_dict *
tomoe_dict_ref (tomoe_dict *dict)
{
    if (!dict) return NULL;
    dict->ref_count++;
    return dict;
}

void
tomoe_dict_unref (tomoe_dict *dict)
{
    if (!dict) return;
    dict->ref_count--;

    if (dict->ref_count <= 0)
        tomoe_dict_free (dict);
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
