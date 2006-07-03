/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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

#include "tomoe-char.h"

#include <stdlib.h>
#include <string.h>

struct _tomoe_char
{
    int           ref;
    char*         charCode;
    tomoe_glyph*  glyph;
    tomoe_array*  readings;
    char*         meta;
};

tomoe_stroke *
tomoe_stroke_new (void)
{
    tomoe_stroke *strk = calloc (1, sizeof (tomoe_stroke));

    if (strk)
        tomoe_stroke_init (strk, 0);

    return strk;
}

void
tomoe_stroke_init (tomoe_stroke *strk, int point_num)
{
    if (!strk) return;

    strk->point_num = point_num;
    strk->points    = calloc (point_num, sizeof (tomoe_point));
}

void
tomoe_stroke_clear (tomoe_stroke *strk)
{
    if (!strk) return;

    if (strk->points != NULL)
    {
        free (strk->points);
        strk->points = NULL;
    }
}

void
tomoe_stroke_free (tomoe_stroke *strk)
{
    if (!strk) return;

    tomoe_stroke_clear (strk);
    free (strk);
}

tomoe_glyph *
tomoe_glyph_new (void)
{
    tomoe_glyph *glyph = calloc (1, sizeof (tomoe_glyph));

    if (glyph)
        tomoe_glyph_init (glyph, 0);

    return glyph;
}

void
tomoe_glyph_init (tomoe_glyph *glyph, int stroke_num)
{
    if (!glyph) return;

    glyph->stroke_num = stroke_num;
    glyph->strokes    = calloc (stroke_num, sizeof (tomoe_stroke));
}

void
tomoe_glyph_clear (tomoe_glyph *glyph)
{
    unsigned int i;

    if (!glyph) return;

    if (glyph->strokes) {
        for (i = 0; i < glyph->stroke_num; i++)
            tomoe_stroke_clear (&glyph->strokes[i]);
        free (glyph->strokes);
        glyph->strokes = NULL;
    }

    glyph->stroke_num = 0;
}

void
tomoe_glyph_free (tomoe_glyph *glyph)
{
    if (!glyph) return;

    tomoe_glyph_clear (glyph);
    free (glyph);
}

tomoe_char*
tomoe_char_new (void)
{
    tomoe_char *p = calloc (1, sizeof (tomoe_char));
    if (!p) return NULL;

    p->ref       = 1;
    p->charCode  = NULL;
    p->glyph     = NULL;
    p->meta      = NULL;
    p->readings  = NULL;

    return p;
}

tomoe_char*
tomoe_char_addref (tomoe_char* this)
{
    if (!this) return NULL;
    this->ref ++;
    return this;
}

void
tomoe_char_free (tomoe_char *this)
{
    if (!this) return;

    this->ref --;
    if (this->ref <= 0)
    {
        free (this->charCode);
        tomoe_glyph_free (this->glyph);
        free (this->meta);
        free (this);
    }
}

const char*
tomoe_char_getCode (const tomoe_char* this)
{
    if (!this) return NULL;
    return this->charCode;
}

void
tomoe_char_setCode (tomoe_char* this, const char* code)
{
    if (!this) return;
    free (this->charCode);
    this->charCode = strdup (code);
}

tomoe_array*
tomoe_char_getReadings (tomoe_char* this)
{
    if (!this) return NULL;
    return this->readings;
}

void
tomoe_char_setReadings (tomoe_char* this, tomoe_array* readings)
{
    if (!this) return;
    tomoe_array_free (this->readings);
    this->readings = tomoe_array_addref (readings);
}

tomoe_glyph*
tomoe_char_getGlyph (tomoe_char* this)
{
    if (!this) return NULL;
    return this->glyph;
}

void
tomoe_char_setGlyph (tomoe_char* this, tomoe_glyph* glyph)
{
    if (!this) return;
    tomoe_glyph_free (this->glyph);
    this->glyph = glyph;
}

const char*
tomoe_char_getMeta (const tomoe_char* this)
{
    if (!this) return NULL;
    return this->meta;
}

void
tomoe_char_setMeta (tomoe_char* this, const char* meta)
{
    if (!this) return;
    free (this->meta);
    this->meta = strdup (meta);
}

int
tomoe_char_compare (const tomoe_char** a, const tomoe_char** b)
{
    return strcmp ((*a)->charCode, (*b)->charCode);
}

int
tomoe_candidate_compare (const tomoe_candidate** a, const tomoe_candidate** b)
{
    int score_a = a[0]->score;
    int score_b = b[0]->score;

    return score_a > score_b ? 1
        : score_a < score_b ? -1
        : 0;
}

int
tomoe_string_compare (const char** a, const char** b)
{
    return strcmp(*a, *b);
}
