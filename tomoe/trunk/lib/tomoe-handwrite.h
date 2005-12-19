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

#ifndef __TOMOE_HANDWRITE_H__
#define __TOMOE_HANDWRITE_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "tomoe-data-types.h"
#include "tomoe-dict.h"

typedef struct _tomoe_hw_context tomoe_hw_context;

tomoe_hw_context   *tomoe_hw_context_new        (void);
void                tomoe_hw_context_free       (tomoe_hw_context *ctx);

void                tomoe_hw_append_dictionary  (tomoe_hw_context *ctx,
                                                 tomoe_dict       *dict);
void                tomoe_hw_remove_dictionary  (tomoe_hw_context *ctx,
                                                 tomoe_dict       *dict);
const tomoe_dict  **tomoe_hw_get_dictionaries   (tomoe_hw_context *ctx);

void                tomoe_hw_set_canvas_width   (tomoe_hw_context *ctx,
                                                 unsigned int      width);
void                tomoe_hw_set_canvas_height  (tomoe_hw_context *ctx,
                                                 unsigned int      height);
unsigned int        tomoe_hw_get_canvas_width   (tomoe_hw_context *ctx);
unsigned int        tomoe_hw_get_canvas_height  (tomoe_hw_context *ctx);

void                tomoe_hw_append_point       (tomoe_hw_context *ctx,
                                                 unsigned int      x,
                                                 unsigned int      y);
void                tomoe_hw_commit_stroke      (tomoe_hw_context *ctx);
void                tomoe_hw_remove_prev_stroke (tomoe_hw_context *ctx);
void                tomeo_hw_clear_stroke       (tomoe_hw_context *ctx);
unsigned int        tomoe_hw_get_n_stroke       (tomoe_hw_context *ctx);
const tomoe_glyph  *tomoe_hw_get_glyph          (tomoe_hw_context *ctx);

const tomoe_candidate
                  **tomoe_hw_get_candidates     (tomoe_hw_context *ctx);

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_HANDWRITE_H__ */
