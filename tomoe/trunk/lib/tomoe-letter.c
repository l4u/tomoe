/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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

#include "tomoe-letter.h"

#include <stdlib.h>

void
tomoe_stroke_init (tomoe_stroke *strk)
{
    strk->point_num = 0;
    strk->points    = NULL;
}

void
tomoe_stroke_init_with_points (tomoe_stroke *strk, int point_num)
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

void
tomoe_glyph_init (tomoe_glyph *glyph)
{
    if (!glyph) return;

    glyph->stroke_num = 0;
    glyph->strokes    = NULL;
}

void
tomoe_glyph_init_with_strokes (tomoe_glyph *glyph, int stroke_num)
{
    if (!glyph) return;

    glyph->stroke_num = stroke_num;
    glyph->strokes    = calloc (stroke_num, sizeof (tomoe_stroke));
}

void
tomoe_glyph_clear (tomoe_glyph *glyph)
{
    int i;

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

void
tomoe_letter_clear (tomoe_letter *lttr)
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
            tomoe_stroke_clear (&lttr->c_glyph->strokes[i]);
        free (lttr->c_glyph->strokes);
        free (lttr->c_glyph);
        lttr->c_glyph->strokes = NULL;
        lttr->c_glyph          = NULL;
    }
}
