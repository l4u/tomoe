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

#include <stdlib.h>

#include <gmodule.h>
#include "tomoe-recognizer.h"
#include "tomoe-recognizer-impl.h"

#define RECOGNIZER_NEW_FUNC "tomoe_recognizer_impl_new"
#define RECOGNIZER_FREE_FUNC "tomoe_recognizer_impl_free"
#define RECOGNIZER_SEARCH_FUNC "tomoe_recognizer_impl_search"

G_DEFINE_TYPE (TomoeRecognizer, tomoe_recognizer, TOMOE_TYPE_MODULE)

static void
tomoe_recognizer_class_init (TomoeRecognizerClass *klass)
{
}

static void
tomoe_recognizer_init (TomoeRecognizer *recognizer)
{
}

TomoeRecognizer *
tomoe_recognizer_new (const gchar *base_dir, const gchar *name)
{
    TomoeRecognizer *recognizer;

    recognizer = g_object_new (TOMOE_TYPE_RECOGNIZER,
                               "default_base_dir", RECOGNIZERDIR,
                               "new_func_name", RECOGNIZER_NEW_FUNC,
                               "free_func_name", RECOGNIZER_FREE_FUNC,
                               NULL);
    tomoe_module_find_module (TOMOE_MODULE (recognizer), base_dir, name);
    return recognizer;
}

GList *
tomoe_recognizer_search (const TomoeRecognizer *recognizer,
                         TomoeDict *dict, TomoeWriting *input)
{
    TomoeModule *module;
    GList *result = NULL;
    TomoeRecognizerSearchFunc search_func;
    TomoeRecognizerSearchFunc *search_func_p;
    gpointer *p;

    g_return_val_if_fail (TOMOE_IS_RECOGNIZER (recognizer), NULL);
    module = TOMOE_MODULE (recognizer);
    search_func_p = &search_func;
    p = (gpointer *)search_func_p;
    if (tomoe_module_load_func (module, RECOGNIZER_SEARCH_FUNC, p)) {
        result = search_func (tomoe_module_get_context (module), dict, input);
    } else {
        tomoe_module_show_error (module);
    }

    return result;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
