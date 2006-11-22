/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
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

#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "tomoe-char.h"
#include "tomoe-dict.h"
#include "glib-utils.h"

#define TOMOE_CHAR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_CHAR, TomoeCharPrivate))

typedef struct _TomoeCharPrivate	TomoeCharPrivate;
struct _TomoeCharPrivate
{
    char                 *charCode;
    TomoeGlyph           *glyph;
    GPtrArray            *readings;
    GHashTable           *meta;
};

enum
{
  PROP_0
};

G_DEFINE_TYPE (TomoeChar, tomoe_char, G_TYPE_OBJECT)

static void tomoe_char_dispose        (GObject         *object);
static void tomoe_char_set_property   (GObject         *object,
                                       guint            prop_id,
                                       const GValue    *value,
                                       GParamSpec      *pspec);
static void tomoe_char_get_property   (GObject         *object,
                                       guint            prop_id,
                                       GValue          *value,
                                       GParamSpec      *pspec);

static void
tomoe_char_class_init (TomoeCharClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose  = tomoe_char_dispose;
    gobject_class->set_property = tomoe_char_set_property;
    gobject_class->get_property = tomoe_char_get_property;

    g_type_class_add_private (gobject_class, sizeof (TomoeCharPrivate));
}

static void
tomoe_char_init (TomoeChar *t_char)
{
    TomoeCharPrivate *priv = TOMOE_CHAR_GET_PRIVATE (t_char);
    priv->charCode  = NULL;
    priv->glyph     = NULL;
    priv->meta      = g_hash_table_new_full(g_str_hash, g_str_equal,
                                            g_free, g_free);
    priv->readings  = NULL;
}

TomoeChar*
tomoe_char_new (void)
{
    return g_object_new(TOMOE_TYPE_CHAR, NULL);
}

static void
tomoe_char_dispose (GObject *object)
{
    TomoeCharPrivate *priv = TOMOE_CHAR_GET_PRIVATE (object);

    if (priv->charCode)
        g_free (priv->charCode);
    if (priv->glyph)
        g_object_unref (G_OBJECT (priv->glyph));
    if (priv->meta)
        g_hash_table_destroy (priv->meta);
    if (priv->readings)
        TOMOE_PTR_ARRAY_FREE_ALL (priv->readings, g_free);

    priv->charCode = NULL;
    priv->glyph    = NULL;
    priv->meta     = NULL;
    priv->readings = NULL;

    G_OBJECT_CLASS (tomoe_char_parent_class)->dispose (object);
}
static void
tomoe_char_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    TomoeChar *t_char;
    TomoeCharPrivate *priv;

    t_char = TOMOE_CHAR(object);
    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
tomoe_char_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    TomoeChar *t_char;
    TomoeCharPrivate *priv;

    t_char = TOMOE_CHAR (object);
    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

const char*
tomoe_char_get_code (const TomoeChar* t_char)
{
    TomoeCharPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CHAR (t_char), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);
    return priv->charCode;
}

void
tomoe_char_set_code (TomoeChar* t_char, const char* code)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (t_char));

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);
    g_free (priv->charCode);
    priv->charCode = code ? g_strdup (code) : NULL;
}

static void
_copy_reading_func (gpointer data, gpointer user_data)
{
    GPtrArray *new = (GPtrArray *) user_data;
    const gchar *reading = (const gchar*) data;

    g_ptr_array_add (new, g_strdup (reading));
}

#warning FIXME! this interface is too bad. We need TomoeCharReading object?
GPtrArray*
tomoe_char_get_readings (TomoeChar* t_char)
{
    TomoeCharPrivate *priv;
    GPtrArray *readings;

    g_return_val_if_fail (TOMOE_IS_CHAR (t_char), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    readings = g_ptr_array_new ();
    if (priv->readings) {
        g_ptr_array_foreach (priv->readings, _copy_reading_func, readings);
    }
    return readings;
}

void
tomoe_char_set_readings (TomoeChar* t_char, GPtrArray* readings)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (t_char));

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    if (priv->readings) {
        TOMOE_PTR_ARRAY_FREE_ALL (priv->readings, g_free);
        priv->readings = NULL;
    }

    priv->readings = g_ptr_array_new ();
    if (readings) {
        g_ptr_array_foreach (readings, _copy_reading_func, priv->readings);
    }
}

TomoeGlyph*
tomoe_char_get_glyph (TomoeChar* t_char)
{
    TomoeCharPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CHAR (t_char), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    return priv->glyph; 
}

void
tomoe_char_set_glyph (TomoeChar* t_char, TomoeGlyph* glyph)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (t_char));

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    if (priv->glyph)
        g_object_unref (G_OBJECT (priv->glyph));
    priv->glyph = g_object_ref (glyph);
}

gint
tomoe_char_compare (const TomoeChar *a, const TomoeChar *b)
{
    TomoeCharPrivate *priv_a, *priv_b;

    if (!a || !b) return 0;

    priv_a = TOMOE_CHAR_GET_PRIVATE (a);
    priv_b = TOMOE_CHAR_GET_PRIVATE (b);
    if (!priv_a || !priv_b) return 0;

    if (!priv_a->charCode || !priv_b->charCode) return 0;
    return strcmp (priv_a->charCode, priv_b->charCode);
}

void
tomoe_char_register_meta_data (TomoeChar *chr, const gchar *key,
                               const gchar *value)
{
    TomoeCharPrivate *priv;
    g_return_if_fail (chr);
    g_return_if_fail (key);
    g_return_if_fail (value);

    priv = TOMOE_CHAR_GET_PRIVATE (chr);
    g_hash_table_insert (priv->meta, g_strdup (key), g_strdup (value));
}

const gchar*
tomoe_char_get_meta_data (TomoeChar* chr, const gchar *key)
{
    TomoeCharPrivate *priv;
    g_return_val_if_fail (chr, NULL);
    g_return_val_if_fail (key, NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (chr);
    return g_hash_table_lookup (priv->meta, key);
}

void
tomoe_char_meta_data_foreach (TomoeChar* chr, GHFunc func, gpointer user_data)
{
    TomoeCharPrivate *priv;
    g_return_if_fail (chr);

    priv = TOMOE_CHAR_GET_PRIVATE (chr);
    g_hash_table_foreach (priv->meta, func, user_data);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
