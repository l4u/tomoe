
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <stdlib.h>

#include "tomoe-module.h"
#include "tomoe-recognizer.h"

static GList *recognizers = NULL;

void
tomoe_recognizer_load (const gchar *base_dir)
{
    if (!base_dir)
        base_dir = g_getenv ("TOMOE_RECOGNIZER_MODULE_DIR");
    if (!base_dir)
        base_dir = RECOGNIZER_MODULEDIR;

    recognizers = g_list_concat (tomoe_module_load_modules (base_dir),
                                 recognizers);
}

void
tomoe_recognizer_unload (void)
{
    g_list_foreach (recognizers, (GFunc) tomoe_module_unload, NULL);
    g_list_free (recognizers);
    recognizers = NULL;
}

GList *
tomoe_recognizer_get_registered_types (void)
{
    return tomoe_module_collect_registered_types (recognizers);
}

GList *
tomoe_recognizer_get_log_domains (void)
{
    return tomoe_module_collect_log_domains (recognizers);
}

G_DEFINE_ABSTRACT_TYPE (TomoeRecognizer, tomoe_recognizer, G_TYPE_OBJECT)

static void
tomoe_recognizer_class_init (TomoeRecognizerClass *klass)
{
    klass->search = NULL;
}

static void
tomoe_recognizer_init (TomoeRecognizer *recognizer)
{
}

/**
 * tomoe_recognizer_new:
 * @name: The name of recognizer type.
 * @first_property: the name of the first property.
 * @... :  the value of the first property, followed optionally by more name/value pairs, followed by NULL
 *
 * Create a new #TomoeRecognizer.
 *
 * Return value: a new #TomoeRecognizer.
 */
TomoeRecognizer *
tomoe_recognizer_new (const gchar *name, const gchar *first_property, ...)
{
    GObject *recognizer;
    va_list var_args;

    va_start (var_args, first_property);
    recognizer = tomoe_module_instantiate (recognizers, name,
                                           first_property, var_args);
    va_end (var_args);

    return TOMOE_RECOGNIZER (recognizer);
}

/**
 * tomoe_recognizer_search:
 * @recognizer: a TomoeRecognizer object.
 * @input: a #TomoeWriting object for matching.
 *
 * Match strokes of TomoeChar with TomoeWriting.
 *
 * Return value: A newly-allocated list of
 * #TomoeCandidate. The each #TomoeCandidate should be also
 * freed with g_object_unref.
  */
GList *
tomoe_recognizer_search (TomoeRecognizer *recognizer, TomoeWriting *input)
{
    TomoeRecognizerClass *klass;

    g_return_val_if_fail (TOMOE_IS_RECOGNIZER (recognizer), NULL);

    klass = TOMOE_RECOGNIZER_GET_CLASS (recognizer);
    if (klass->search)
        return klass->search (recognizer, input);
    else
        return NULL;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
