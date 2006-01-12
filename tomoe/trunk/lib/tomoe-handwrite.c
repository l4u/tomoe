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
#include "tomoe-handwrite.h"

#define DEFAULT_CANVAS_WIDTH  300
#define DEFAULT_CANVAS_HEIGHT 300

struct _tomoe_hw_context
{
    tomoe_dict  **dict;
    unsigned int  dict_num;

    tomoe_glyph  *glyph;

    unsigned int  canvas_width;
    unsigned int  canvas_height;
};

tomoe_hw_context *
tomoe_hw_context_new (void)
{
    tomoe_hw_context *ctx = malloc (sizeof (tomoe_hw_context));
    ctx->dict          = NULL;
    ctx->dict_num      = 0;
    ctx->glyph         = NULL;
    ctx->canvas_width  = DEFAULT_CANVAS_WIDTH;
    ctx->canvas_height = DEFAULT_CANVAS_HEIGHT;
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
    if (!ctx)
        return;
    if (!dict)
        return;

    ctx->dict_num++;
    ctx->dict = realloc (ctx->dict,
                         sizeof (tomoe_dict*) * (ctx->dict_num + 1));
    ctx->dict[ctx->dict_num - 1] = dict;
    ctx->dict[ctx->dict_num] = NULL;
}

void
tomoe_hw_remove_dictionary (tomoe_hw_context *ctx, tomoe_dict *dict)
{
    int i;

    if (!ctx)
        return;
    if (!dict)
        return;

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
    if (!ctx)
        return 0;
    return ctx->dict_num;
}

const tomoe_dict **
tomoe_hw_get_dictionaries (tomoe_hw_context *ctx)
{
    if (!ctx)
        return NULL;
    if (ctx->dict_num <= 0)
        return NULL;
    return (const tomoe_dict**) ctx->dict;
}
