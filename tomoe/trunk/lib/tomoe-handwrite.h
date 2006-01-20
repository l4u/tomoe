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

/**
 *  @file tomoe-handwrite.h
 *  @brief 
 */

#ifndef __TOMOE_HANDWRITE_H__
#define __TOMOE_HANDWRITE_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "tomoe-data-types.h"
#include "tomoe-dict.h"

/**
 * @typedef typedef struct _tomoe_hw_context tomoe_hw_context;
 *
 * A struct type which is used for keeping a context of handwriting. Typically
 * one handwrite canvas will have one tomoe_hw_context.
 */
typedef struct _tomoe_hw_context tomoe_hw_context;

/**
 * @brief Create a tomoe_hw_context struct.
 * @return Pointer to the newly allocated tomoe_hw_context struct.
 */
tomoe_hw_context   *tomoe_hw_context_new        (void);

/**
 * @brief Free an allocated tomoe_hw_context struct.
 * @param ctx - Pointer to the tomoe_hw_context to free.
 */
void                tomoe_hw_context_free       (tomoe_hw_context *ctx);

/**
 * @brief Increase reference count of a tomoe_hw_context.
 * @param - Pointer to the tomoe_hw_context struct to increase reference count.
 * @return The tomoe_hw_context.
 */
tomoe_hw_context   *tomoe_hw_context_ref        (tomoe_hw_context *ctx);

/**
 * @brief Decrease reference count.
 *        If the reference count has reached to 0, the context will be freed
 *        automatically.
 * @param Pointer to the tomoe_hw_context struct to decrease reference count.
 */
void                tomoe_hw_context_unref      (tomoe_hw_context *ctx);

/**
 * @brief Append a tomoe dictionary into a tomoe_hw_context.
 * @param ctx  - The tomoe_hw_context to remove a dictionary.
 * @param dict - A tomoe dictionary to append. NULL pointer will be ignored.
 */
void                tomoe_hw_append_dictionary  (tomoe_hw_context *ctx,
                                                 tomoe_dict       *dict);

/**
 * @brief Remove a tomoe dictionary from a tomoe_hw_context.
 * @param ctx  - The tomoe_hw_context to append a dictionary.
 * @param dict - A otmoe dictionary to remove. NULL pointer will be ignored.
 */
void                tomoe_hw_remove_dictionary  (tomoe_hw_context *ctx,
                                                 tomoe_dict       *dict);

/**
 * @brief Return number of dictionaries which is kept in a tomoe_hw_context.
 * @param ctx - Pointer to the tomoe_hw_context which keeps tomoe dictionaries.
 * @return Number of dictionaries.
 */
unsigned int        tomoe_hw_get_number_of_dictionaries
                                                (tomoe_hw_context *ctx);

/**
 * @brief Return array of tomoe_dict poitners which is kept by a
 *        tomoe_hw_context.
 * @param ctx - Pointer to the tomoe_hw_context which keeps tomoe dictionaries.
 * @return An array of tomoe_dict pointers. The array is terminated by NULL
 *         pointer. NULL will be returned if the context has no dictionary.
 */
const tomoe_dict  **tomoe_hw_get_dictionaries   (tomoe_hw_context *ctx);

void                tomoe_hw_push_point         (tomoe_hw_context *ctx,
                                                 unsigned int      x,
                                                 unsigned int      y);
void                tomoe_hw_pop_point          (tomoe_hw_context *ctx);
void                tomoe_hw_push_stroke        (tomoe_hw_context *ctx);
void                tomoe_hw_pop_stroke         (tomoe_hw_context *ctx);
unsigned int        tomoe_hw_get_number_of_strokes
                                                (tomoe_hw_context *ctx);
const tomoe_glyph  *tomoe_hw_get_glyph          (tomoe_hw_context *ctx);
void                tomeo_hw_clear_glyph        (tomoe_hw_context *ctx);

void                tomoe_hw_set_canvas_width   (tomoe_hw_context *ctx,
                                                 unsigned int      width);
void                tomoe_hw_set_canvas_height  (tomoe_hw_context *ctx,
                                                 unsigned int      height);
unsigned int        tomoe_hw_get_canvas_width   (tomoe_hw_context *ctx);
unsigned int        tomoe_hw_get_canvas_height  (tomoe_hw_context *ctx);

const tomoe_candidate
                  **tomoe_hw_get_candidates     (tomoe_hw_context *ctx);

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_HANDWRITE_H__ */
