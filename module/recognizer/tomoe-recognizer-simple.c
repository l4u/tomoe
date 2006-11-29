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
#include <gmodule.h>

#include <tomoe-module-impl.h>
#include <tomoe-recognizer.h>
#include "tomoe-recognizer-simple-logic.h"

#define TOMOE_TYPE_RECOGNIZER_SIMPLE            tomoe_type_recognizer_simple
#define TOMOE_RECOGNIZER_SIMPLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_RECOGNIZER_SIMPLE, TomoeRecognizerSimple))
#define TOMOE_RECOGNIZER_SIMPLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_RECOGNIZER_SIMPLE, TomoeRecognizerSimpleClass))
#define TOMOE_IS_RECOGNIZER_SIMPLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_RECOGNIZER_SIMPLE))
#define TOMOE_IS_RECOGNIZER_SIMPLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_RECOGNIZER_SIMPLE))
#define TOMOE_RECOGNIZER_SIMPLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_RECOGNIZER_SIMPLE, TomoeRecognizerSimpleClass))


typedef struct _TomoeRecognizerSimple TomoeRecognizerSimple;
typedef struct _TomoeRecognizerSimpleClass TomoeRecognizerSimpleClass;

struct _TomoeRecognizerSimple
{
    TomoeRecognizer object;
};

struct _TomoeRecognizerSimpleClass
{
    TomoeRecognizerClass parent_class;
};

static GType tomoe_type_recognizer_simple = 0;

static GList *search (TomoeRecognizer *recognizer,
                      TomoeDict       *dict,
                      TomoeWriting    *input);

static void
class_init (TomoeRecognizerSimpleClass *klass)
{
    TomoeRecognizerClass *recognizer_class;

    recognizer_class = TOMOE_RECOGNIZER_CLASS (klass);

    recognizer_class->search = search;
}

static void
init (TomoeRecognizerSimple *recognizer)
{
}

static void
register_type (GTypeModule *type_module)
{
    static const GTypeInfo info =
        {
            sizeof (TomoeRecognizerSimpleClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (TomoeRecognizerSimple),
            0,
            (GInstanceInitFunc) init,
        };

    tomoe_type_recognizer_simple =
        g_type_module_register_type (type_module,
                                     TOMOE_TYPE_RECOGNIZER,
                                     "TomoeRecognizerSimple",
                                     &info, 0);
}

G_MODULE_EXPORT void
TOMOE_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    register_type (type_module);
}

G_MODULE_EXPORT void
TOMOE_MODULE_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
TOMOE_MODULE_IMPL_INSTANTIATE (const gchar *first_property, va_list args)
{
    GObject *object;

    object = g_object_new_valist (TOMOE_TYPE_RECOGNIZER_SIMPLE,
                                  first_property, args);

    return object;
}

static GList *
search (TomoeRecognizer *recognizer, TomoeDict *dict, TomoeWriting *input)
{
    return _tomoe_recognizer_simple_get_candidates (recognizer, dict, input);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
