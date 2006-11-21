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
#include "tomoe-config.h"
#include "tomoe-shelf.h"
#include "tomoe-candidate.h"
#include "glib-utils.h"

#define TOMOE_CONTEXT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_CONTEXT, TomoeContextPrivate))

typedef struct _TomoeContextPrivate	TomoeContextPrivate;
struct _TomoeContextPrivate
{
    TomoeShelf *shelf;
    TomoeConfig *config;
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

    priv->shelf      = tomoe_shelf_new ();
    priv->config     = NULL;
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

    if (priv->shelf) {
        g_object_unref (priv->shelf);
    }

    if (priv->config) {
        g_object_unref (priv->config);
    }

    if (priv->recognizer) {
        g_object_unref (priv->recognizer);
    }

    priv->shelf      = NULL;
    priv->config     = NULL;
    priv->recognizer = NULL;

    G_OBJECT_CLASS (tomoe_context_parent_class)->dispose (object);
}

void
tomoe_context_add_dict (TomoeContext *context, TomoeDict *dict)
{
    TomoeContextPrivate *priv;
    g_return_if_fail (context);
    g_return_if_fail (dict);

    priv = TOMOE_CONTEXT_GET_PRIVATE (context);
    tomoe_shelf_add_dict(priv->shelf, dict);
    if (priv->config)
        tomoe_config_save(priv->config);
}

GList *
tomoe_context_get_dict_names (TomoeContext *context)
{
    TomoeContextPrivate *priv;
    g_return_val_if_fail (context, NULL);

    priv = TOMOE_CONTEXT_GET_PRIVATE(context);
    return tomoe_shelf_get_dict_names(priv->shelf);
}

TomoeDict *
tomoe_context_get_dict (TomoeContext *context, const gchar *name)
{
    TomoeContextPrivate *priv;
    g_return_val_if_fail (context, NULL);

    priv = TOMOE_CONTEXT_GET_PRIVATE(context);
    return tomoe_shelf_get_dict(priv->shelf, name);
}

gboolean
tomoe_context_remove_dict (TomoeContext *context, const gchar *name)
{
    TomoeContextPrivate *priv;
    gboolean success = FALSE;
    g_return_val_if_fail (context, FALSE);

    priv = TOMOE_CONTEXT_GET_PRIVATE(context);
    success = tomoe_shelf_remove_dict(priv->shelf, name);
    if (success && priv->config)
        tomoe_config_save(priv->config);
    return success;
}

void
tomoe_context_load_config (TomoeContext *ctx, const char *config_file)
{
    TomoeContextPrivate *priv;
    TomoeConfig* cfg;

    g_return_if_fail (ctx);

    priv = TOMOE_CONTEXT_GET_PRIVATE(ctx);
    priv->config = NULL;
    cfg = tomoe_config_new (config_file);
    tomoe_config_load (cfg);
    tomoe_config_setup_context (cfg, ctx);
    priv->config = cfg;
}

void
tomoe_context_save (TomoeContext *context)
{
    TomoeContextPrivate *priv;

    g_return_if_fail (context);
    priv = TOMOE_CONTEXT_GET_PRIVATE (context);

    tomoe_shelf_save(priv->shelf);
    if (priv->config)
        tomoe_config_save(priv->config);
}

#warning FIXME!
static void
_ptr_array_merge_func (gpointer data, gpointer user_data)
{
    GPtrArray *p = (GPtrArray *) user_data;
    g_ptr_array_add (p, data);
}

static gint
_candidate_compare_func (gconstpointer a, gconstpointer b)
{
    TomoeCandidate *ca = *(TomoeCandidate **) a;
    TomoeCandidate *cb = *(TomoeCandidate **) b;
    return tomoe_candidate_compare (ca, cb);
}

GList *
tomoe_context_search_by_strokes (TomoeContext *context, TomoeGlyph *input)
{
    TomoeContextPrivate *priv;
    GList *names, *name;
    GList *matched = NULL;

    if (!context) return matched;

    names = tomoe_context_get_dict_names(context);
    if (!names) return matched;

    priv = TOMOE_CONTEXT_GET_PRIVATE (context);
    for (name = names; name; name = name->next) {
        TomoeDict *dict;

        dict = tomoe_context_get_dict(context, name->data);
        matched = g_list_concat (tomoe_recognizer_search (priv->recognizer,
                                                          dict, input),
                                 matched);
    }
    matched = g_list_sort (matched, _candidate_compare_func);

    return matched;
}

GPtrArray *
tomoe_context_search_by_reading (TomoeContext *context, const char *input)
{
    GList *names, *name;
    GPtrArray *reading = g_ptr_array_new ();

    if (!context) return reading;

    names = tomoe_context_get_dict_names(context);
    if (!names) return reading;

    for (name = names; name; name = name->next) {
        GPtrArray *tmp;
        TomoeDict *dict;
        dict = tomoe_context_get_dict(context, name->data);
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
