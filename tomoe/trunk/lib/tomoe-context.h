/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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

/**
 *  @file tomoe-context.h
 *  @brief Provides access to the tomoe context.
 */

#ifndef __TOMOE_CONTEXT_H__
#define __TOMOE_CONTEXT_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "tomoe-data-types.h"
#include "tomoe-dict.h"
#include "tomoe-config.h"

/**
 * @typedef typedef struct _TomoeContext TomoeContext;
 *
 * A struct type which represents tomoe database. All members in it should be
 * accessed through tomoe_dict_* functions.
 */
typedef struct _TomoeContext TomoeContext;

/**
 * @brief Create a database.
 * @return Pointer to newly allocated TomoeContext struct.
 */
TomoeContext   *tomoe_context_new                    (void);

/**
 * @brief Increase reference count.
 * @param ctx - Pointer to the TomoeContext struct to increase reference count.
 * @return The TomoeContext.
 */
TomoeContext   *tomoe_context_add_ref                (TomoeContext       *ctx);

/**
 * @brief Decrease reference count and free if zero.
 * @param ctx - Pointer to the TomoeContext struct to free.
 */
void            tomoe_context_free                   (TomoeContext       *ctx);

/**
 * @brief Load dictionary into database.
 * @param ctx - Pointer to the TomoeContext struct to increase reference count.
 * @param filename - Name of dictionary file to load.
 * @return The TomoeContext.
 */
void             tomoe_context_add_dict               (TomoeContext       *ctx,
                                                      TomoeDict          *dict);
void             tomoe_context_load_dict              (TomoeContext       *ctx,
                                                      const char         *filename,
                                                      int                 editable);
void             tomoe_context_load_dict_list         (TomoeContext       *ctx,
                                                      const GPtrArray    *list);
const GPtrArray *tomoe_context_get_dict_list          (TomoeContext       *ctx);
void             tomoe_context_save                   (TomoeContext       *ctx);

#if 0
TomoeDict      *tomoe_context_get_user_dict          (TomoeContext       *ctx,
                                                      const char    *filename);
#endif

/**
 * @brief Match strokes of tomoe_letter with input.
 * @param ctx      - Pointer to the TomoeContext object.
 * @param input    - Pointer to tomoe_glyph matchkey.
 * @return The array of tomoe_candidate.
 */
GPtrArray      *tomoe_context_search_by_strokes      (TomoeContext       *ctx,
                                                      TomoeGlyph         *input);

/**
 * @brief Match reading of tomoe_letter with input.
 * @param ctx     - Pointer to the TomoeContext object.
 * @param reading  - Pointer to string matchkey
 * @return The array of tomoe_candidate.
 */
GPtrArray      *tomoe_context_search_by_reading      (TomoeContext       *ctx,
                                                      const char         *reading);
#if 0
/* optional */
void            tomoe_context_enable_dict            (TomoeContext       *ctx,
                                                      const char    *filename);
void            tomoe_context_disable_dict           (TomoeContext       *ctx,
                                                      const char    *filename);
int             tomoe_context_is_dict_enabled        (TomoeContext       *ctx,
                                                      const char    *filename);
TomoeArray*     tomoe_context_get_dicts              (TomoeContext       *ctx);
#endif

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_CONTEXT_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
