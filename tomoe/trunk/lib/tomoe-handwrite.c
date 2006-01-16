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

#include <stdlib.h>
#include <string.h>
#include "tomoe-handwrite.h"

#define DEFAULT_CANVAS_WIDTH  300
#define DEFAULT_CANVAS_HEIGHT 300

struct _tomoe_hw_context
{
    tomoe_dict  **dict;
    unsigned int  dict_num;

    tomoe_glyph  *glyph;
    tomoe_glyph  *normalized_glyph;

    unsigned int  canvas_width;
    unsigned int  canvas_height;

    tomoe_bool    stroke_is_pending;
};

tomoe_hw_context *
tomoe_hw_context_new (void)
{
    tomoe_hw_context *ctx = malloc (sizeof (tomoe_hw_context));
    ctx->dict              = NULL;
    ctx->dict_num          = 0;
    ctx->glyph             = tomoe_glyph_new ();
    ctx->normalized_glyph  = tomoe_glyph_new ();
    ctx->canvas_width      = DEFAULT_CANVAS_WIDTH;
    ctx->canvas_height     = DEFAULT_CANVAS_HEIGHT;
    ctx->stroke_is_pending = FALSE;
    return ctx;
}

void
tomoe_hw_context_free (tomoe_hw_context *ctx)
{
    int i;

    for (i = 0; ctx->dict && ctx->dict[i]; i++)
        tomoe_dict_free (ctx->dict[i]);
    free (ctx->dict);

    tomoe_glyph_free (ctx->glyph);
    free (ctx);
}

void
tomoe_hw_append_dictionary (tomoe_hw_context *ctx, tomoe_dict *dict)
{
    if (!ctx) return;
    if (!dict) return;

    ctx->dict_num++;
    ctx->dict = realloc (ctx->dict,
                         sizeof (tomoe_dict*) * (ctx->dict_num + 1));
    ctx->dict[ctx->dict_num - 1] = dict;
    ctx->dict[ctx->dict_num] = NULL;
}

void
tomoe_hw_remove_dictionary (tomoe_hw_context *ctx, tomoe_dict *dict)
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
                 sizeof (tomoe_dict*) * ctx->dict_num - i);
        ctx->dict_num--;
        ctx->dict = realloc (ctx->dict,
                             sizeof (tomoe_dict*) * (ctx->dict_num + 1));
    }
}

unsigned int
tomoe_hw_get_number_of_dictionaries (tomoe_hw_context *ctx)
{
    if (!ctx) return 0;
    return ctx->dict_num;
}

const tomoe_dict **
tomoe_hw_get_dictionaries (tomoe_hw_context *ctx)
{
    if (!ctx) return NULL;
    if (ctx->dict_num < 1) return NULL;
    return (const tomoe_dict**) ctx->dict;
}

void
tomoe_hw_push_point (tomoe_hw_context *ctx, unsigned int x, unsigned int y)
{
    tomoe_stroke *stroke = NULL;
    tomoe_point *point = NULL;

    if (!ctx) return;

    if (!ctx->stroke_is_pending) {
        ctx->glyph->stroke_num++;
        ctx->glyph->strokes
            = realloc (ctx->glyph->strokes,
                       sizeof (tomoe_stroke) * ctx->glyph->stroke_num);
        stroke = &ctx->glyph->strokes[ctx->glyph->stroke_num - 1];
        tomoe_stroke_init (stroke);
        ctx->stroke_is_pending = TRUE;
    } else {
        stroke = &ctx->glyph->strokes[ctx->glyph->stroke_num - 1];
    }

    stroke->point_num++;
    stroke->points = realloc (stroke->points,
                              sizeof (tomoe_stroke) * stroke->point_num);
    point = &stroke->points[stroke->point_num - 1];

    point->x = x;
    point->y = y;
}

void
tomoe_hw_pop_point (tomoe_hw_context *ctx)
{
    tomoe_stroke *stroke = NULL;

    if (!ctx) return;
    if (!ctx->stroke_is_pending) return;
    if (ctx->glyph->stroke_num < 1) return;

    stroke = &ctx->glyph->strokes[ctx->glyph->stroke_num - 1];

    stroke->point_num--;
    stroke->points = realloc (stroke->points,
                              sizeof (tomoe_stroke) * stroke->point_num);

    if (stroke->point_num < 1 || !stroke->points) {
        ctx->glyph->stroke_num--;
        ctx->glyph->strokes
            = realloc (ctx->glyph->strokes,
                       sizeof (tomoe_stroke) * ctx->glyph->stroke_num);
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
                   sizeof (tomoe_stroke) * ctx->glyph->stroke_num);
    ctx->stroke_is_pending = FALSE;
}

unsigned int
tomoe_hw_get_number_of_strokes (tomoe_hw_context *ctx)
{
    if (!ctx) return 0;
    if (!ctx->glyph) return 0;
    return ctx->glyph->stroke_num;
}

const tomoe_glyph *
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

const tomoe_candidate **
tomoe_hw_get_candidates (tomoe_hw_context *ctx)
{
    if (!ctx) return NULL;

    return NULL;
}
