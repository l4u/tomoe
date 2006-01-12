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

struct _tomoe_hw_context
{
    tomoe_dict  **dict;
    tomoe_glyph  *glyph;
};

tomoe_hw_context *
tomoe_hw_context_new (void)
{
    tomoe_hw_context *ctx = malloc (sizeof (tomoe_hw_context));
    ctx->dict  = NULL;
    ctx->glyph = NULL;
    return ctx;
}

void
tomoe_hw_context_free (tomoe_hw_context *ctx)
{
    int i;

    for (i = 0; ctx->dict && ctx->dict[i]; i++) {
        // free contents
    }
    free (ctx->dict);

    tomoe_glyph_free (ctx->glyph);
    free (ctx);
}
