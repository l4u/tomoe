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

enum {
    PROP_0,
    PROP_RECOGNIZER
};

typedef struct _TomoeContextPrivate	TomoeContextPrivate;
struct _TomoeContextPrivate
{
    TomoeShelf *shelf;
    TomoeConfig *config;
    TomoeRecognizer *recognizer;
};

G_DEFINE_TYPE (TomoeContext, tomoe_context, G_TYPE_OBJECT)

static void         dispose                   (GObject       *object);
static void         set_property              (GObject       *object,
                                               guint         prop_id,
                                               const GValue  *value,
                                               GParamSpec    *pspec);
static void         get_property              (GObject       *object,
                                               guint          prop_id,
                                               GValue        *value,
                                               GParamSpec    *pspec);

static void
tomoe_context_class_init (TomoeContextClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose      = dispose;
    gobject_class->set_property = set_property;
    gobject_class->get_property = get_property;

    g_object_class_install_property (
        gobject_class,
        PROP_RECOGNIZER,
        g_param_spec_object (
            "recognizer",
            "Recognizer",
            "The recognizer of the context",
            TOMOE_TYPE_RECOGNIZER,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_type_class_add_private (gobject_class, sizeof (TomoeContextPrivate));
}

static void
tomoe_context_init (TomoeContext *context)
{
    TomoeContextPrivate *priv = TOMOE_CONTEXT_GET_PRIVATE (context);

    priv->shelf      = NULL;
    priv->config     = NULL;
    priv->recognizer = NULL;
}

/**
 * tomoe_context_new:
 * 
 * Create a new #TomoeContext.
 *
 * Return value: a new #TomoeContext.
 */
TomoeContext*
tomoe_context_new(void)
{
    TomoeContext *context;

    context = g_object_new(TOMOE_TYPE_CONTEXT, NULL);

    return context;
}

static void
set_property (GObject *object,
              guint prop_id,
              const GValue *value,
              GParamSpec *pspec)
{
    TomoeContextPrivate *priv = TOMOE_CONTEXT_GET_PRIVATE (object);

    switch (prop_id) {
      case PROP_RECOGNIZER:
        if (priv->recognizer)
            g_object_unref (priv->recognizer);
        priv->recognizer = g_value_get_object (value);
        if (priv->recognizer)
            g_object_ref (priv->recognizer);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}


static void
get_property (GObject *object,
              guint prop_id,
              GValue *value,
              GParamSpec *pspec)
{
    TomoeContextPrivate *priv = TOMOE_CONTEXT_GET_PRIVATE (object);

    switch (prop_id) {
      case PROP_RECOGNIZER:
        g_value_set_object (value, priv->recognizer);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
dispose (GObject *object)
{
    TomoeContextPrivate *priv = TOMOE_CONTEXT_GET_PRIVATE (object);

    if (priv->shelf)
        g_object_unref (priv->shelf);
    if (priv->config)
        g_object_unref (priv->config);
    if (priv->recognizer)
        g_object_unref (priv->recognizer);

    priv->shelf      = NULL;
    priv->config     = NULL;
    priv->recognizer = NULL;

    G_OBJECT_CLASS (tomoe_context_parent_class)->dispose (object);
}

/**
 * tomoe_context_load_config:
 * @ctx: a #TomoeContext.
 * @config_file: the filename of configuration file to load.
 * 
 * Load dictionaries into configuration file.
 */
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
    priv->shelf = tomoe_config_make_shelf (cfg);
    priv->config = cfg;
}

static gint
_candidate_compare_func (gconstpointer a, gconstpointer b)
{
    TomoeCandidate *ca = *(TomoeCandidate **) a;
    TomoeCandidate *cb = *(TomoeCandidate **) b;
    return tomoe_candidate_compare (ca, cb);
}


static GList *
tomoe_context_search_by_strokes (TomoeContext *context, TomoeWriting *input)
{
    TomoeContextPrivate *priv;
    GList *matched = NULL;

    g_return_val_if_fail (context, matched);
    if (!input) return matched;

    priv = TOMOE_CONTEXT_GET_PRIVATE (context);
    if (!priv->recognizer) {
        TomoeShelf *shelf;
        TomoeDict *dict;

        shelf = priv->shelf;
        g_return_val_if_fail (shelf, matched);

        dict = tomoe_shelf_get_dict (shelf, "TOMOE Strokelist Dictionary");
        g_return_val_if_fail (dict, matched);

        priv->recognizer = tomoe_recognizer_new ("simple",
                                                 "dictionary", dict,
                                                 NULL);
    }

    matched = g_list_sort (tomoe_recognizer_search (priv->recognizer, input),
                           _candidate_compare_func);

    return matched;
}

static GList *
tomoe_context_search_by_dict (TomoeContext *context, TomoeQuery *query)
{
    TomoeContextPrivate *priv;
    TomoeShelf *shelf;
    GList *names, *name;
    GList *results = NULL;

    if (!context) return results;

    priv = TOMOE_CONTEXT_GET_PRIVATE (context);
    shelf = priv->shelf;
    if (!shelf) return results;

    names = tomoe_shelf_get_dict_names(shelf);
    if (!names) return results;

    for (name = names; name; name = name->next) {
        TomoeDict *dict;
        dict = tomoe_shelf_get_dict(shelf, name->data);
        results = g_list_concat (tomoe_dict_search (dict, query), results);
    }
    results = g_list_sort (results, _candidate_compare_func);

    return results;
}

GList *
tomoe_context_search (TomoeContext *context, TomoeQuery *query)
{
    TomoeWriting *writing;

    writing = tomoe_query_get_writing (query);
    if (writing)
        return tomoe_context_search_by_strokes (context, writing);
    else
        return tomoe_context_search_by_dict (context, query);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
