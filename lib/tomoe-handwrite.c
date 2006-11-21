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

#include <stdlib.h>
#include "tomoe-handwrite.h"

TomoeStroke *
tomoe_stroke_new (void)
{
    TomoeStroke *strk = calloc (1, sizeof (TomoeStroke));

    if (strk)
        tomoe_stroke_init (strk, 0);

    return strk;
}

void
tomoe_stroke_init (TomoeStroke *strk, int point_num)
{
    if (!strk) return;

    strk->point_num = point_num;
    strk->points    = calloc (point_num, sizeof (TomoePoint));
}

void
tomoe_stroke_clear (TomoeStroke *strk)
{
    if (!strk) return;

    if (strk->points != NULL) {
        free (strk->points);
        strk->points = NULL;
    }
}

void
tomoe_stroke_free (TomoeStroke *strk)
{
    if (!strk) return;

    tomoe_stroke_clear (strk);
    free (strk);
}

TomoeGlyph *
tomoe_glyph_new (void)
{
    TomoeGlyph *glyph = calloc (1, sizeof (TomoeGlyph));

    if (glyph)
        tomoe_glyph_init (glyph, 0);

    return glyph;
}

void
tomoe_glyph_init (TomoeGlyph *glyph, int stroke_num)
{
    if (!glyph) return;

    glyph->stroke_num = stroke_num;
    glyph->strokes    = calloc (stroke_num, sizeof (TomoeStroke));
}

void
tomoe_glyph_clear (TomoeGlyph *glyph)
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
tomoe_glyph_free (TomoeGlyph *glyph)
{
    if (!glyph) return;

    tomoe_glyph_clear (glyph);
    free (glyph);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/