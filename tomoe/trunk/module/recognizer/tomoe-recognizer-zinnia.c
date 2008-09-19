/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2008 Kouhei Sutou <kou@cozmixng.org>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <gmodule.h>

#include <glib/gi18n-lib.h>

#include <tomoe-module-impl.h>
#include <tomoe-recognizer.h>
#include <tomoe-candidate.h>

#include <zinnia.h>

#define TOMOE_TYPE_RECOGNIZER_ZINNIA            tomoe_type_recognizer_zinnia
#define TOMOE_RECOGNIZER_ZINNIA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_RECOGNIZER_ZINNIA, TomoeRecognizerZinnia))
#define TOMOE_RECOGNIZER_ZINNIA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_RECOGNIZER_ZINNIA, TomoeRecognizerZinniaClass))
#define TOMOE_IS_RECOGNIZER_ZINNIA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_RECOGNIZER_ZINNIA))
#define TOMOE_IS_RECOGNIZER_ZINNIA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_RECOGNIZER_ZINNIA))
#define TOMOE_RECOGNIZER_ZINNIA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_RECOGNIZER_ZINNIA, TomoeRecognizerZinniaClass))

typedef struct _TomoeRecognizerZinnia TomoeRecognizerZinnia;
typedef struct _TomoeRecognizerZinniaClass TomoeRecognizerZinniaClass;

struct _TomoeRecognizerZinnia
{
    TomoeRecognizer  object;
    zinnia_recognizer_t *zinnia;
};

struct _TomoeRecognizerZinniaClass
{
    TomoeRecognizerClass parent_class;
};

static GType tomoe_type_recognizer_zinnia = 0;
static GObjectClass *parent_class;

static GObject     *constructor               (GType                  type,
                                               guint                  n_props,
                                               GObjectConstructParam *props);
static void         dispose                   (GObject       *object);
static GList       *search                    (TomoeRecognizer *recognizer,
                                               TomoeWriting    *input);
static gboolean     is_available              (TomoeRecognizer *recognizer);

static void
class_init (TomoeRecognizerZinniaClass *klass)
{
    GObjectClass *gobject_class;
    TomoeRecognizerClass *recognizer_class;

    parent_class = g_type_class_peek_parent (klass);

    gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->constructor  = constructor;
    gobject_class->dispose      = dispose;

    recognizer_class = TOMOE_RECOGNIZER_CLASS (klass);
    recognizer_class->search = search;
    recognizer_class->is_available = is_available;
}

static void
init (TomoeRecognizerZinnia *recognizer)
{
    TomoeRecognizer *_recognizer;

    _recognizer = TOMOE_RECOGNIZER (recognizer);
    recognizer->zinnia = NULL;
}

static void
register_type (GTypeModule *type_module)
{
    static const GTypeInfo info =
        {
            sizeof (TomoeRecognizerZinniaClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (TomoeRecognizerZinnia),
            0,
            (GInstanceInitFunc) init,
        };

    tomoe_type_recognizer_zinnia =
        g_type_module_register_type (type_module,
                                     TOMOE_TYPE_RECOGNIZER,
                                     "TomoeRecognizerZinnia",
                                     &info, 0);
}

G_MODULE_EXPORT GList *
TOMOE_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    GList *registered_types = NULL;

    register_type (type_module);
    if (tomoe_type_recognizer_zinnia) {
        gchar *name = (gchar *) g_type_name (tomoe_type_recognizer_zinnia);
        registered_types = g_list_prepend (registered_types, name);
    }

    return registered_types;
}

G_MODULE_EXPORT void
TOMOE_MODULE_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
TOMOE_MODULE_IMPL_INSTANTIATE (const gchar *first_property, va_list var_args)
{
    return g_object_new_valist (TOMOE_TYPE_RECOGNIZER_ZINNIA,
                                first_property, var_args);
}

G_MODULE_EXPORT gchar *
TOMOE_MODULE_IMPL_GET_LOG_DOMAIN (void)
{
    return g_strdup (G_LOG_DOMAIN);
}

static gboolean
open_recognizer (zinnia_recognizer_t *zinnia, const gchar *language)
{
    gboolean success;
    gchar *model_name;
    gchar *filename;

    if (language)
        model_name = g_strconcat ("handwriting-", language, ".model", NULL);
    else
        model_name = g_strdup ("handwriting.model");

    filename = g_build_filename (RECOGNIZER_DATADIR, model_name, NULL);
    success = zinnia_recognizer_open (zinnia, filename);
    g_free(model_name);
    g_free(filename);

    return success;
}

static GObject *
constructor (GType type, guint n_props,
             GObjectConstructParam *props)
{
    GObject *object;
    GObjectClass *klass = G_OBJECT_CLASS (parent_class);
    TomoeRecognizerZinnia *recognizer;

    object = klass->constructor (type, n_props, props);
    recognizer = TOMOE_RECOGNIZER_ZINNIA (object);

    if (!recognizer->zinnia) {
        TomoeRecognizer *_recognizer;

        recognizer->zinnia = zinnia_recognizer_new();
        _recognizer = TOMOE_RECOGNIZER (object);
        if (!open_recognizer (recognizer->zinnia,
                              tomoe_recognizer_get_language (_recognizer))) {
            zinnia_recognizer_destroy (recognizer->zinnia);
            recognizer->zinnia = NULL;
        }
    }

    return object;
}

static void
dispose (GObject *object)
{
    TomoeRecognizerZinnia *recognizer;

    recognizer = TOMOE_RECOGNIZER_ZINNIA (object);

    if (recognizer->zinnia) {
        zinnia_recognizer_destroy (recognizer->zinnia);
        recognizer->zinnia = NULL;
    }

    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static GList *
search (TomoeRecognizer *_recognizer, TomoeWriting *input)
{
    TomoeRecognizerZinnia *recognizer;
    zinnia_character_t *character;
    zinnia_result_t *result;
    const GList *strokes;
    GList *candidates = NULL;
    gsize i, result_size;

    character = zinnia_character_new ();
    zinnia_character_clear (character);
    zinnia_character_set_width (character, TOMOE_WRITING_WIDTH);
    zinnia_character_set_height (character, TOMOE_WRITING_HEIGHT);

    for (strokes = tomoe_writing_get_strokes (input), i = 0;
         strokes;
         strokes = g_list_next (strokes), i++) {
        GList *points;

        for (points = strokes->data; points; points = g_list_next (points)) {
            TomoePoint *point = points->data;

            zinnia_character_add (character, i, point->x, point->y);
        }
    }

    recognizer = TOMOE_RECOGNIZER_ZINNIA (_recognizer);
    result = zinnia_recognizer_classify (recognizer->zinnia, character, 10);
    zinnia_character_destroy (character);

    if (!result)
        return NULL;

    result_size = zinnia_result_size (result);
    for (i = 0; i < result_size; i++) {
        TomoeCandidate *candidate;
        TomoeChar *character;

        character = tomoe_char_new ();
        tomoe_char_set_utf8 (character, zinnia_result_value (result, i));

        candidate = tomoe_candidate_new (character);
        g_object_unref (character);
        tomoe_candidate_set_score (candidate, zinnia_result_score (result, i));

        candidates = g_list_prepend (candidates, candidate);
    }
    zinnia_result_destroy (result);

    return g_list_reverse (candidates);
}

static gboolean
is_available (TomoeRecognizer *_recognizer)
{
    TomoeRecognizerZinnia *recognizer;

    recognizer = TOMOE_RECOGNIZER_ZINNIA (_recognizer);
    return recognizer->zinnia != NULL;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
