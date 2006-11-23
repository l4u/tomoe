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
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>

#include "tomoe-enum-types.h"
#include "tomoe-query.h"
#include "glib-utils.h"

#define TOMOE_QUERY_GET_PRIVATE(obj) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_QUERY, TomoeQueryPrivate))

typedef struct _TomoeQueryPrivate	TomoeQueryPrivate;
struct _TomoeQueryPrivate
{
    GList        *readings;
    GList        *radicals;
    GList        *variants;
    gint          number_of_strokes;
    TomoeWriting *writing;
};

enum
{
    PROP_0,
    PROP_READINGS,
    PROP_RADICALS,
    PROP_VARIANTS,
    PROP_NUMBER_OF_STROKES,
    PROP_WRITING
};

G_DEFINE_TYPE (TomoeQuery, tomoe_query, G_TYPE_OBJECT)

static void tomoe_query_dispose        (GObject         *object);
static void tomoe_query_set_property   (GObject         *object,
                                        guint            prop_id,
                                        const GValue    *value,
                                        GParamSpec      *pspec);
static void tomoe_query_get_property   (GObject         *object,
                                        guint            prop_id,
                                        GValue          *value,
                                        GParamSpec      *pspec);

static void
tomoe_query_class_init (TomoeQueryClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose  = tomoe_query_dispose;
    gobject_class->set_property = tomoe_query_set_property;
    gobject_class->get_property = tomoe_query_get_property;

    g_type_class_add_private (gobject_class, sizeof (TomoeQueryPrivate));
}

static void
tomoe_query_init (TomoeQuery *query)
{
    TomoeQueryPrivate *priv = TOMOE_QUERY_GET_PRIVATE (query);

    priv->readings = NULL;
    priv->radicals = NULL;
    priv->variants = NULL;
    priv->number_of_strokes = -1;
    priv->writing = NULL;
}

TomoeQuery *
tomoe_query_new (void)
{
    return g_object_new(TOMOE_TYPE_QUERY, NULL);
}

static void
tomoe_query_dispose (GObject *object)
{
    TomoeQueryPrivate *priv = TOMOE_QUERY_GET_PRIVATE (object);

    if (priv->readings) {
        g_list_foreach (priv->readings, (GFunc)g_object_unref, NULL);
        g_list_free (priv->readings);
    }
    if (priv->radicals) {
        g_list_foreach (priv->radicals, (GFunc)g_object_unref, NULL);
        g_list_free (priv->radicals);
    }
    if (priv->variants) {
        g_list_foreach (priv->variants, (GFunc)g_object_unref, NULL);
        g_list_free (priv->variants);
    }

    if (priv->writing)
        g_object_unref (priv->writing);

    priv->readings = NULL;
    priv->radicals = NULL;
    priv->variants = NULL;
    priv->number_of_strokes = -1;
    priv->writing = NULL;

    G_OBJECT_CLASS (tomoe_query_parent_class)->dispose (object);
}
static void
tomoe_query_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    TomoeQuery *query;
    TomoeQueryPrivate *priv;

    query = TOMOE_QUERY(object);
    priv = TOMOE_QUERY_GET_PRIVATE (query);

    switch (prop_id) {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
tomoe_query_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    TomoeQuery *query;
    TomoeQueryPrivate *priv;

    query = TOMOE_QUERY (object);
    priv = TOMOE_QUERY_GET_PRIVATE (query);

    switch (prop_id) {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

void
tomoe_query_add_reading (TomoeQuery* query, TomoeReading *reading)
{
    TomoeQueryPrivate *priv;

    g_return_if_fail (TOMOE_IS_QUERY (query));

    priv = TOMOE_QUERY_GET_PRIVATE (query);
    priv->readings = g_list_append (priv->readings, g_object_ref (reading));
}

const GList *
tomoe_query_get_readings (TomoeQuery* query)
{
    TomoeQueryPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_QUERY (query), NULL);

    priv = TOMOE_QUERY_GET_PRIVATE (query);
    return priv->readings;
}


void
tomoe_query_set_writing (TomoeQuery* query, TomoeWriting *writing)
{
    TomoeQueryPrivate *priv;

    g_return_if_fail (TOMOE_IS_QUERY (query));

    priv = TOMOE_QUERY_GET_PRIVATE (query);
    if (priv->writing)
        g_object_unref (priv->writing);

    priv->writing = g_object_ref (writing);
}

TomoeWriting *
tomoe_query_get_writing (TomoeQuery* query)
{
    TomoeQueryPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_QUERY (query), NULL);

    priv = TOMOE_QUERY_GET_PRIVATE (query);
    return priv->writing;
}


/*
vi:ts=4:nowrap:ai:expandtab
*/