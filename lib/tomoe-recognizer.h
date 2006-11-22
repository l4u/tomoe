/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Kouhei Sutou <kou@cozmixng.org>
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
 *  @file tomoe-recognizer.h
 *  @brief Provide a set of API to recognize handwriting.
 */

#ifndef __TOMOE_RECOGNIZER_H__
#define __TOMOE_RECOGNIZER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#include <tomoe-dict.h>
#include <tomoe-writing.h>

#define TOMOE_TYPE_RECOGNIZER            (tomoe_recognizer_get_type ())
#define TOMOE_RECOGNIZER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_RECOGNIZER, TomoeRecognizer))
#define TOMOE_RECOGNIZER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_RECOGNIZER, TomoeRecognizerClass))
#define TOMOE_IS_RECOGNIZER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_RECOGNIZER))
#define TOMOE_IS_RECOGNIZER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_RECOGNIZER))
#define TOMOE_RECOGNIZER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_RECOGNIZER, TomoeRecognizerClass))

typedef struct _TomoeRecognizer TomoeRecognizer;
typedef struct _TomoeRecognizerClass TomoeRecognizerClass;

struct _TomoeRecognizer
{
    GObject object;
};

struct _TomoeRecognizerClass
{
    GObjectClass parent_class;
};

GType            tomoe_recognizer_get_type (void) G_GNUC_CONST;
TomoeRecognizer *tomoe_recognizer_new      (void);

/**
 * @brief Match strokes of tomoe_char with input.
 * @param recognizer   - Pointer to the TomoeRecognizer object.
 * @param dict   - Pointer to the TomoeDict object.
 * @param input  - Pointer to tomoe_glyph matchkey.
 * @return The array of TomoeCandidate.
 */
GList           *tomoe_recognizer_search   (const TomoeRecognizer *recognizer,
                                            TomoeDict *dict,
                                            TomoeGlyph *inputs);

G_END_DECLS

#endif /* __TOMOE_RECOGNIZER_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
