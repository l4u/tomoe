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
    gchar                *char_code;
    TomoeWriting         *writing;
    GList                *readings;
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
tomoe_char_init (TomoeChar *chr)
{
    TomoeCharPrivate *priv = TOMOE_CHAR_GET_PRIVATE (chr);
    priv->char_code  = NULL;
    priv->writing    = NULL;
    priv->meta       = g_hash_table_new_full(g_str_hash, g_str_equal,
                                             g_free, g_free);
    priv->readings   = NULL;
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

    if (priv->char_code)
        g_free (priv->char_code);
    if (priv->writing)
        g_object_unref (G_OBJECT (priv->writing));
    if (priv->meta)
        g_hash_table_destroy (priv->meta);
    if (priv->readings) {
        g_list_foreach (priv->readings, (GFunc)g_object_unref, NULL);
        g_list_free (priv->readings);
    }

    priv->char_code = NULL;
    priv->writing   = NULL;
    priv->meta      = NULL;
    priv->readings  = NULL;

    G_OBJECT_CLASS (tomoe_char_parent_class)->dispose (object);
}
static void
tomoe_char_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    TomoeChar *chr;
    TomoeCharPrivate *priv;

    chr = TOMOE_CHAR(object);
    priv = TOMOE_CHAR_GET_PRIVATE (chr);

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
    TomoeChar *chr;
    TomoeCharPrivate *priv;

    chr = TOMOE_CHAR (object);
    priv = TOMOE_CHAR_GET_PRIVATE (chr);

    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

const gchar *
tomoe_char_get_code (TomoeChar* chr)
{
    TomoeCharPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CHAR (chr), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (chr);
    return priv->char_code;
}

void
tomoe_char_set_code (TomoeChar* chr, const char* code)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (chr));

    priv = TOMOE_CHAR_GET_PRIVATE (chr);
    g_free (priv->char_code);
    priv->char_code = code ? g_strdup (code) : NULL;
}

const GList *
tomoe_char_get_readings (TomoeChar* chr)
{
    TomoeCharPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CHAR (chr), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (chr);
    return priv->readings;
}

void
tomoe_char_add_reading (TomoeChar* chr, TomoeReading *reading)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (chr));

    priv = TOMOE_CHAR_GET_PRIVATE (chr);

    priv->readings = g_list_prepend(priv->readings, g_object_ref(reading));
}

TomoeWriting*
tomoe_char_get_writing (TomoeChar* chr)
{
    TomoeCharPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CHAR (chr), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (chr);

    return priv->writing; 
}

void
tomoe_char_set_writing (TomoeChar* chr, TomoeWriting* writing)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (chr));

    priv = TOMOE_CHAR_GET_PRIVATE (chr);

    if (priv->writing)
        g_object_unref (G_OBJECT (priv->writing));
    priv->writing = g_object_ref (writing);
}

gint
tomoe_char_compare (const TomoeChar *a, const TomoeChar *b)
{
    TomoeCharPrivate *priv_a, *priv_b;

    if (!a || !b) return 0;

    priv_a = TOMOE_CHAR_GET_PRIVATE (a);
    priv_b = TOMOE_CHAR_GET_PRIVATE (b);
    if (!priv_a || !priv_b) return 0;

    if (!priv_a->char_code || !priv_b->char_code) return 0;
    return strcmp (priv_a->char_code, priv_b->char_code);
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

gboolean
tomoe_char_has_meta_data (TomoeChar *chr)
{
    TomoeCharPrivate *priv;
    g_return_val_if_fail (chr, FALSE);

    priv = TOMOE_CHAR_GET_PRIVATE (chr);
    return g_hash_table_size (priv->meta) > 0;
}

void
tomoe_char_meta_data_foreach (TomoeChar* chr, GHFunc func, gpointer user_data)
{
    TomoeCharPrivate *priv;
    g_return_if_fail (chr);

    priv = TOMOE_CHAR_GET_PRIVATE (chr);
    g_hash_table_foreach (priv->meta, func, user_data);
}


static void
tomoe_char_to_xml_char_code (TomoeChar *chr, TomoeCharPrivate *priv,
                             GString *output)
{
    gchar *code_point;

    if (!priv->char_code) return;

    code_point = g_markup_printf_escaped ("    <code-point>%s</code-point>\n",
                                          priv->char_code);
    g_string_append (output, code_point);
    g_free (code_point);
}

static void
tomoe_char_to_xml_readings (TomoeChar *chr, TomoeCharPrivate *priv,
                            GString *output)
{
    GList *node;

    if (!priv->readings) return;

    g_string_append (output, "    <readings>\n");
    for (node = priv->readings; node; node = g_list_next (node)) {
        TomoeReading *reading = node->data;
        gchar *xml;

        if (!TOMOE_IS_READING (reading)) continue;

        xml = g_markup_printf_escaped ("      <reading>%s</reading>\n",
                                       tomoe_reading_get_reading (reading));
        g_string_append (output, xml);
        g_free (xml);
    }
    g_string_append (output, "    </readings>\n");
}

static void
tomoe_char_to_xml_writing (TomoeChar *chr, TomoeCharPrivate *priv,
                           GString *output)
{
    gchar *xml;

    if (!priv->writing) return;

    xml = tomoe_writing_to_xml (priv->writing);

    if (xml && xml[0] != '\0') {
        g_string_append (output, xml);
        g_free (xml);
    }
}


static void
tomoe_char_to_xml_meta_datum (gpointer key, gpointer value, gpointer user_data)
{
    GString *output = user_data;
    gchar *meta_key = key;
    gchar *meta_value = value;
    gchar *result;

    result = g_markup_printf_escaped ("      <%s>%s</%s>\n",
                                      meta_key, meta_value, meta_key);
    g_string_append (output, result);
    g_free (result);
}

static void
tomoe_char_to_xml_meta (TomoeChar *chr, TomoeCharPrivate *priv, GString *output)
{
    if (!tomoe_char_has_meta_data (chr)) return;

    g_string_append (output, "    <meta>\n");
    tomoe_char_meta_data_foreach (chr, tomoe_char_to_xml_meta_datum, output);
    g_string_append (output, "    </meta>\n");
}

gchar *
tomoe_char_to_xml (TomoeChar* chr)
{
    TomoeCharPrivate *priv;
    GString *output;

    g_return_val_if_fail (TOMOE_IS_CHAR (chr), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (chr);
    output = g_string_new ("");

    tomoe_char_to_xml_char_code (chr, priv, output);
    tomoe_char_to_xml_readings (chr, priv, output);
    tomoe_char_to_xml_writing (chr, priv, output);
    tomoe_char_to_xml_meta (chr, priv, output);

    if (output->len > 0) {
        g_string_prepend (output, "  <character>\n");
        g_string_append (output, "  </character>\n");
    }

    return g_string_free (output, FALSE);
}


/*
vi:ts=4:nowrap:ai:expandtab
*/
