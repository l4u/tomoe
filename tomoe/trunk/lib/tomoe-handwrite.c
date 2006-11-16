/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005-2006 Takuro Ashie <ashie@homa.ne.jp>
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
#include <string.h>
#include "tomoe-handwrite.h"

#define DEFAULT_CANVAS_WIDTH  300
#define DEFAULT_CANVAS_HEIGHT 300

struct _tomoe_hw_context
{
    TomoeDict   **dict;
    unsigned int  dict_num;

    TomoeGlyph   *glyph;
    TomoeGlyph   *normalized_glyph;

    unsigned int  canvas_width;
    unsigned int  canvas_height;

    tomoe_bool    stroke_is_pending;

    unsigned int  ref_count;
};

static void normalize_strokes (tomoe_hw_context *ctx);
static void search_glyph      (tomoe_hw_context *ctx,
                               TomoeDict        *dict);

tomoe_hw_context *
tomoe_hw_context_new (void)
{
    tomoe_hw_context *ctx  = malloc (sizeof (tomoe_hw_context));
    ctx->dict              = NULL;
    ctx->dict_num          = 0;
    ctx->glyph             = tomoe_glyph_new ();
    ctx->normalized_glyph  = tomoe_glyph_new ();
    ctx->canvas_width      = DEFAULT_CANVAS_WIDTH;
    ctx->canvas_height     = DEFAULT_CANVAS_HEIGHT;
    ctx->stroke_is_pending = FALSE;
    ctx->ref_count         = 1;
    return ctx;
}

void
tomoe_hw_context_free (tomoe_hw_context *ctx)
{
    unsigned int i;

    for (i = 0; ctx->dict && ctx->dict[i]; i++)
        tomoe_dict_free (ctx->dict[i]);
    free (ctx->dict);

    tomoe_glyph_free (ctx->glyph);
    tomoe_glyph_free (ctx->normalized_glyph);
    free (ctx);
}

tomoe_hw_context *
tomoe_hw_context_ref (tomoe_hw_context *ctx)
{
    if (!ctx) return NULL;
    ctx->ref_count++;
    return ctx;
}

void
tomoe_hw_context_unref (tomoe_hw_context *ctx)
{
    if (!ctx) return;
    ctx->ref_count--;

    if (ctx->ref_count <= 0)
        tomoe_hw_context_free (ctx);
}

void
tomoe_hw_append_dictionary (tomoe_hw_context *ctx, TomoeDict *dict)
{
    if (!ctx) return;
    if (!dict) return;

    ctx->dict_num++;
    ctx->dict = realloc (ctx->dict,
                         sizeof (TomoeDict*) * (ctx->dict_num + 1));
    ctx->dict[ctx->dict_num - 1] = dict;
    ctx->dict[ctx->dict_num] = NULL;
}

void
tomoe_hw_remove_dictionary (tomoe_hw_context *ctx, TomoeDict *dict)
{
    unsigned int i;

    if (!ctx) return;
    if (!dict) return;

    for (i = 0; i < ctx->dict_num && ctx->dict[i]; i++)
    {
        if (ctx->dict[i] != dict)
            continue;

        memmove (ctx->dict + i,
                 ctx->dict + i + 1,
                 sizeof (TomoeDict*) * ctx->dict_num - i);
        ctx->dict_num--;
        ctx->dict = realloc (ctx->dict,
                             sizeof (TomoeDict*) * (ctx->dict_num + 1));
    }
}

unsigned int
tomoe_hw_get_number_of_dictionaries (tomoe_hw_context *ctx)
{
    if (!ctx) return 0;
    return ctx->dict_num;
}

const TomoeDict **
tomoe_hw_get_dictionaries (tomoe_hw_context *ctx)
{
    if (!ctx) return NULL;
    if (ctx->dict_num < 1) return NULL;
    return (const TomoeDict**) ctx->dict;
}

void
tomoe_hw_push_point (tomoe_hw_context *ctx, unsigned int x, unsigned int y)
{
    TomoeStroke *stroke = NULL;
    TomoePoint  *point = NULL;

    if (!ctx) return;

    if (!ctx->stroke_is_pending) {
        ctx->glyph->stroke_num++;
        ctx->glyph->strokes
            = realloc (ctx->glyph->strokes,
                       sizeof (TomoeStroke) * ctx->glyph->stroke_num);
        stroke = &ctx->glyph->strokes[ctx->glyph->stroke_num - 1];
        tomoe_stroke_init (stroke, 0);
        ctx->stroke_is_pending = TRUE;
    } else {
        stroke = &ctx->glyph->strokes[ctx->glyph->stroke_num - 1];
    }

    stroke->point_num++;
    stroke->points = realloc (stroke->points,
                              sizeof (TomoeStroke) * stroke->point_num);
    point = &stroke->points[stroke->point_num - 1];

    point->x = x;
    point->y = y;
}

void
tomoe_hw_pop_point (tomoe_hw_context *ctx)
{
    TomoeStroke *stroke = NULL;

    if (!ctx) return;
    if (!ctx->stroke_is_pending) return;
    if (ctx->glyph->stroke_num < 1) return;

    stroke = &ctx->glyph->strokes[ctx->glyph->stroke_num - 1];

    stroke->point_num--;
    stroke->points = realloc (stroke->points,
                              sizeof (TomoeStroke) * stroke->point_num);

    if (stroke->point_num < 1 || !stroke->points) {
        ctx->glyph->stroke_num--;
        ctx->glyph->strokes
            = realloc (ctx->glyph->strokes,
                       sizeof (TomoeStroke) * ctx->glyph->stroke_num);
        ctx->stroke_is_pending = FALSE;
    }
}

void
tomoe_hw_push_stroke (tomoe_hw_context *ctx)
{
    if (!ctx) return;
    if (!ctx->stroke_is_pending) return;
    ctx->stroke_is_pending = FALSE;
}

void
tomoe_hw_pop_stroke (tomoe_hw_context *ctx)
{
    if (!ctx) return;
    if (ctx->glyph->stroke_num < 1) return;

    ctx->glyph->stroke_num--;
    ctx->glyph->strokes
        = realloc (ctx->glyph->strokes,
                   sizeof (TomoeStroke) * ctx->glyph->stroke_num);
    ctx->stroke_is_pending = FALSE;
}

unsigned int
tomoe_hw_get_number_of_strokes (tomoe_hw_context *ctx)
{
    if (!ctx) return 0;
    if (!ctx->glyph) return 0;
    return ctx->glyph->stroke_num;
}

const TomoeGlyph *
tomoe_hw_get_glyph (tomoe_hw_context *ctx)
{
    if (!ctx) return NULL;
    return ctx->glyph;
}

void
tomeo_hw_clear_glyph (tomoe_hw_context *ctx)
{
    if (!ctx) return;
    if (!ctx->glyph) return;
    tomoe_glyph_clear (ctx->glyph);
}

void
tomoe_hw_set_canvas_width   (tomoe_hw_context *ctx, unsigned int width)
{
    if (!ctx) return;
    if (width <= 0) return;
    ctx->canvas_width = width;
}

void
tomoe_hw_set_canvas_height (tomoe_hw_context *ctx, unsigned int height)
{
    if (!ctx) return;
    if (height <= 0) return;
    ctx->canvas_height = height;
}

unsigned int
tomoe_hw_get_canvas_width (tomoe_hw_context *ctx)
{
    if (!ctx) return 0;
    return ctx->canvas_width;
}

unsigned int
tomoe_hw_get_canvas_height (tomoe_hw_context *ctx)
{
    if (!ctx) return 0;
    return ctx->canvas_height;
}

const TomoeCandidate **
tomoe_hw_get_candidates (tomoe_hw_context *ctx)
{
    unsigned int i;

    if (!ctx) return NULL;

    normalize_strokes (ctx);

    for (i = 0; i < ctx->dict_num; i++)
        search_glyph (ctx, ctx->dict[i]);

    return NULL;
}



/*
 * Functions for normalizing handwrited glyph.
 */
static unsigned int
get_distance (TomoePoint *first, TomoePoint *last, TomoePoint **most)
{
    /*
     * Getting distance 
     * MAX( |aw - bv + c| )
     *   * first = (p, q),  last = (x, y),  other = (v, w)
     *   * a = x - p,   b = y - q,   c = py - qx
     */
    int a, b, c;
    unsigned int dist = 0;
    unsigned int max  = 0;
    unsigned int denominator;
    TomoePoint *p;

    *most = NULL;
    if (first == last)
    {
        return 0;
    }

    a = last->x - first->x;
    b = last->y - first->y;
    c = last->y * first->x - last->x * first->y;

    for (p = first; p < last; p++)
    {
        dist = abs ((a * p->y) - (b * p->x) + c);
        if (dist > max)
        {
            max = dist;
            *most = p;
        }
    }

    denominator = a * a + b * b;

    if (denominator == 0)
        return 0;
    else
        return max * max / denominator;
}

static void
get_vertex (TomoeStroke *dest, TomoePoint *first, TomoePoint *last)
{
    TomoePoint *most = NULL;
    unsigned int dist;
    unsigned int error = 300 * 300 / 400; /* 5% */ /* FIXME! */

    dist = get_distance (first, last, &most);

    if (most && dist > error)
    {
        /*
         * Pick up most far point, and continue to investigate points between
         * first and most. The most far points will be added after it, and
         * investigate points between most and last.
         */
        get_vertex (dest, first, most);
        get_vertex (dest, most, last);
    }
    else
    {
        /*
         * Now we can ignore points between first and last. Most far point from
         * the line is near enough. Simply add last point to dest array.
         */
        /* tomoe_stroke_push_point (dest, last->x, last->y) */
        dest->point_num++;
        dest->points = realloc (dest->points, dest->point_num);
        dest->points[dest->point_num - 1] = *last;
    }
}

static void
normalize_strokes (tomoe_hw_context *ctx)
{
    unsigned int i;

    if (!ctx) return;

    tomoe_glyph_clear (ctx->normalized_glyph);
    tomoe_glyph_init (ctx->normalized_glyph,
                      ctx->glyph->stroke_num);

    for (i = 0; i < ctx->glyph->stroke_num; i++)
    {
        TomoeStroke *dest = &ctx->normalized_glyph->strokes[i];
        TomoeStroke *stroke = &ctx->glyph->strokes[i];

        /* First point is always used. */
        tomoe_stroke_init (dest, 1);
        dest->points[0] = stroke->points[0];

        /* Drop needless points */
        get_vertex (dest,
                    &stroke->points[0],
                    &stroke->points[stroke->point_num - 1]);
    }
}



/*
 * Functions for matching handwrited glyph with registered letters in
 * a dictionary.
 */
static void
search_glyph (tomoe_hw_context *ctx, TomoeDict *dict)
{
    if (!ctx) return;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
