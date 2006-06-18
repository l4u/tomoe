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

#include "tomoe-letter.h"

#include <stdlib.h>
#include <string.h>

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

tomoe_letter*
tomoe_letter_new (void)
{
    tomoe_letter *lttr = calloc (1, sizeof (tomoe_letter));
    if (!lttr) return NULL;

    lttr->ref       = 1;
    lttr->character = NULL;
    lttr->c_glyph   = NULL;
    lttr->meta      = NULL;
    lttr->readings  = tomoe_array_new ((tomoe_compare_fn)tomoe_string_compare,
                                       NULL,
                                       (tomoe_free_fn)free);

    return lttr;
}

tomoe_letter*
tomoe_letter_addref (tomoe_letter* this)
{
    if (!this) return NULL;
    this->ref ++;
    return this;
}

void
tomoe_letter_free (tomoe_letter *this)
{
    if (!this) return;

    this->ref --;
    if (this->ref <= 0)
    {
        free (this->character);
        tomoe_glyph_free (this->c_glyph);
        free (this->meta);
        free (this);
    }
}

int
tomoe_letter_compare (const tomoe_letter** p0, const tomoe_letter** p1)
{
    return 0;
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
