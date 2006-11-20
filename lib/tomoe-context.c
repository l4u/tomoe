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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void tomoe_context_finalize     (GObject *object);
static void tomoe_context_dispose      (GObject *object);

static void
tomoe_context_class_init (TomoeContextClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = tomoe_context_finalize;
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
_dict_free (gpointer data, gpointer user_data)
{
    TomoeDict *dict = (TomoeDict *) data;

    tomoe_dict_free (dict);
}

static void
tomoe_context_dispose (GObject *object)
{
    TomoeContextPrivate *priv = TOMOE_CONTEXT_GET_PRIVATE (object);

    if (priv->dicts) {
        TOMOE_PTR_ARRAY_FREE_ALL (priv->dicts, _dict_free);
    }

    if (priv->recognizer) {
        g_object_unref (priv->recognizer);
    }

    priv->dicts      = NULL;
    priv->recognizer = NULL;

    G_OBJECT_CLASS (tomoe_context_parent_class)->dispose (object);
}

static void
tomoe_context_finalize (GObject *object)
{
    TomoeContext *context;
    context = TOMOE_CONTEXT (object);

    G_OBJECT_CLASS (tomoe_context_parent_class)->finalize (object);
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

    fprintf (stdout, "load dictionary '%s' editable: %s...",
             filename, editable ? "yes" : "no");
    fflush (stdout);
    dict = tomoe_dict_new (filename, editable);
    if (dict) {
        tomoe_context_add_dict (context, dict);
        tomoe_dict_free (dict);
    }
    printf (" ok\n");
}

void
tomoe_context_load_dict_list (TomoeContext *context, const GPtrArray *list)
{
    int i;
    for (i = 0; i < list->len; i++) {
        TomoeDictCfg* p = g_ptr_array_index (list, i);
        if (p->dontLoad) continue;

        if (p->user) {
            tomoe_context_load_dict (context, p->filename, p->writeAccess);
        } else {
            char *file = calloc (strlen (p->filename) +
                                 strlen (TOMOEDATADIR) + 2,
                                 sizeof (char));
            strcpy (file, TOMOEDATADIR);
            strcat (file, "/");
            strcat (file, p->filename);
            tomoe_context_load_dict (context, file, p->writeAccess);
            free (file);
        }
    }
}

const GPtrArray*
tomoe_context_get_dict_list (TomoeContext* context)
{
    TomoeContextPrivate *priv;
    priv = TOMOE_CONTEXT_GET_PRIVATE (context);
    if (!context) return NULL;
    return priv->dicts;
}

void
tomoe_context_save (TomoeContext *context)
{
    TomoeContextPrivate *priv = TOMOE_CONTEXT_GET_PRIVATE (context);
    guint i;

    if (!context) return;

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
