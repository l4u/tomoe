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

#include "tomoe-dict.h"
#include "tomoe-recognizer.h"
#include "tomoe-context.h"
#include "glib-utils.h"

#define TOMOE_CONTEXT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_CONTEXT, TomoeContextPrivate))

typedef struct _TomoeContextPrivate	TomoeContextPrivate;
struct _TomoeContextPrivate
{
    GPtrArray  *dicts;
    TomoeRecognizer *recognizer;
};

G_DEFINE_TYPE (TomoeContext, tomoe_context, G_TYPE_OBJECT)

static void tomoe_context_dispose      (GObject *object);

static void
tomoe_context_class_init (TomoeContextClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose  = tomoe_context_dispose;

    g_type_class_add_private (gobject_class, sizeof (TomoeContextPrivate));
}

static void
tomoe_context_init (TomoeContext *context)
{
    TomoeContextPrivate *priv = TOMOE_CONTEXT_GET_PRIVATE (context);

    priv->dicts      = g_ptr_array_new ();
    priv->recognizer = tomoe_recognizer_new ();
}

TomoeContext*
tomoe_context_new(void)
{
    TomoeContext *context;

    context = g_object_new(TOMOE_TYPE_CONTEXT, NULL);

    return context;
}

static void
tomoe_context_dispose (GObject *object)
{
    TomoeContextPrivate *priv = TOMOE_CONTEXT_GET_PRIVATE (object);

    if (priv->dicts) {
        TOMOE_PTR_ARRAY_FREE_ALL (priv->dicts, g_object_unref);
    }

    if (priv->recognizer) {
        g_object_unref (priv->recognizer);
    }

    priv->dicts      = NULL;
    priv->recognizer = NULL;

    G_OBJECT_CLASS (tomoe_context_parent_class)->dispose (object);
}

void
tomoe_context_add_dict (TomoeContext *context, TomoeDict *dict)
{
    TomoeContextPrivate *priv;
    g_return_if_fail (context || dict);
    priv = TOMOE_CONTEXT_GET_PRIVATE (context);
    g_ptr_array_add (priv->dicts, g_object_ref (dict));
}

void
tomoe_context_load_dict (TomoeContext *context, const char *filename, int editable)
{
    TomoeDict* dict;

    if (!context) return;
    if (!filename) return;

    dict = tomoe_dict_new (filename, editable);
    if (dict) {
        tomoe_context_add_dict (context, dict);
        g_object_unref (dict);
    }
}

void
tomoe_context_load_dict_list (TomoeContext *context, const GPtrArray *list)
{
    gint i;
    for (i = 0; i < list->len; i++) {
        TomoeDictCfg* p = g_ptr_array_index (list, i);
        if (p->dontLoad) continue;

        if (p->user) {
            tomoe_context_load_dict (context, p->filename, p->writeAccess);
        } else {
            gchar *filename = g_build_filename (TOMOEDATADIR, p->filename, NULL);
            tomoe_context_load_dict (context, filename, p->writeAccess);
            g_free (filename);
        }
    }
}

const GPtrArray*
tomoe_context_get_dict_list (TomoeContext* context)
{
    TomoeContextPrivate *priv;
    g_return_val_if_fail (context, NULL);

    priv = TOMOE_CONTEXT_GET_PRIVATE (context);
    return priv->dicts;
}

void
tomoe_context_save (TomoeContext *context)
{
    TomoeContextPrivate *priv;
    guint i;

    g_return_if_fail (context);
    priv = TOMOE_CONTEXT_GET_PRIVATE (context);

    for (i = 0; i < priv->dicts->len; i++) {
        TomoeDict *dict = (TomoeDict*) g_ptr_array_index (priv->dicts, i);
        if (tomoe_dict_is_modified (dict))
            tomoe_dict_save (dict);
    }
}

#warning FIXME!
static void
_ptr_array_merge_func (gpointer data, gpointer user_data)
{
    GPtrArray *p = (GPtrArray *) user_data;
    g_ptr_array_add (p, data);
}

static void
_candidate_merge_func (gpointer data, gpointer user_data)
{
    GPtrArray *p = (GPtrArray *) user_data;
    g_ptr_array_add (p, tomoe_candidate_add_ref ((TomoeCandidate *) data));
}

static gint
_candidate_compare_func (gconstpointer a, gconstpointer b)
{
    TomoeCandidate *ca = *(TomoeCandidate **) a;
    TomoeCandidate *cb = *(TomoeCandidate **) b;
    return tomoe_candidate_compare (ca, cb);
}

GPtrArray *
tomoe_context_search_by_strokes (TomoeContext *context, TomoeGlyph *input)
{
    TomoeContextPrivate *priv;
    guint i, num;
    TomoeDict *dict;
    GPtrArray *matched = g_ptr_array_new ();

    if (!context) return matched;
    priv = TOMOE_CONTEXT_GET_PRIVATE (context);
    num = priv->dicts->len;
    if (num == 0) return matched;

    for (i = 0; i < num; i++) {
        GPtrArray *tmp;
        dict = (TomoeDict*)g_ptr_array_index (priv->dicts, i);
        tmp = tomoe_recognizer_search (priv->recognizer, dict, input);

        if (tmp) {
            g_ptr_array_foreach (tmp, _candidate_merge_func, matched);
            g_ptr_array_free (tmp, TRUE);
        }
    }
    g_ptr_array_sort (matched, _candidate_compare_func);

    return matched;
}

GPtrArray *
tomoe_context_search_by_reading (TomoeContext *context, const char *input)
{
    guint i, num;
    TomoeContextPrivate *priv;
    GPtrArray *reading = g_ptr_array_new ();

    if (!context) return reading;
    priv = TOMOE_CONTEXT_GET_PRIVATE (context);
    num = priv->dicts->len;
    if (num == 0) return reading;

    for (i = 0; i < num; i++) {
        GPtrArray *tmp;
    	TomoeDict *dict;
        dict = (TomoeDict*) g_ptr_array_index (priv->dicts, i);
        tmp = tomoe_dict_search_by_reading (dict, input);
        if (tmp) {
            g_ptr_array_foreach (tmp, _ptr_array_merge_func, reading);
            g_ptr_array_free (tmp, TRUE);
        }
    }
    /*tomoe_array_sort (reading);*/

    return reading;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
