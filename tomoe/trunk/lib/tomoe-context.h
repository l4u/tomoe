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

#include <glib-object.h>

G_BEGIN_DECLS

#include "tomoe-dict.h"
#include "tomoe-config.h"

#define TOMOE_TYPE_CONTEXT            (tomoe_context_get_type ())
#define TOMOE_CONTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_CONTEXT, TomoeContext))
#define TOMOE_CONTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_CONTEXT, TomoeContextClass))
#define TOMOE_IS_CONTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_CONTEXT))
#define TOMOE_IS_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_CONTEXT))
#define TOMOE_CONTEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_CONTEXT, TomoeContextClass))

typedef struct _TomoeContext TomoeContext;
typedef struct _TomoeContextClass TomoeContextClass;

struct _TomoeContext
{
    GObject object;
};

struct _TomoeContextClass
{
    GObjectClass parent_class;
};

GType            tomoe_context_get_type (void) G_GNUC_CONST;

/**
 * @brief Create a database.
 * @return Pointer to newly allocated TomoeContext struct.
 */
TomoeContext    *tomoe_context_new      (void);

/**
 * @brief Load dictionary into database.
 * @param ctx - Pointer to the TomoeContext struct to increase reference count.
 * @param filename - Name of dictionary file to load.
 * @return The TomoeContext.
 */
void             tomoe_context_add_dict       (TomoeContext       *ctx,
                                               TomoeDict          *dict);
void             tomoe_context_load_config    (TomoeContext       *ctx,
                                               const gchar        *config_file);
void             tomoe_context_save           (TomoeContext       *ctx);


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
G_END_DECLS

#endif /* __TOMOE_CONTEXT_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
