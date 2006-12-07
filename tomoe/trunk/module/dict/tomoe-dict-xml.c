/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2000 - 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gmodule.h>

#include <tomoe-module-impl.h>
#include <tomoe-dict.h>
#include <tomoe-candidate.h>
#include <tomoe-xml-parser.h>
#include <glib-utils.h>

#include "tomoe-dict-ptr-array.h"
#include "tomoe-dict-xml.h"

#define TOMOE_DICT_XML_GET_PRIVATE(obj) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_DICT_XML, TomoeDictXMLPrivate))

typedef struct _TomoeDictXMLPrivate	TomoeDictXMLPrivate;
struct _TomoeDictXMLPrivate
{
    gchar               *filename;
    gchar               *name;
    GPtrArray           *chars;

    gboolean             editable;
    gboolean             modified;
};

enum {
    PROP_0,
    PROP_FILENAME,
    PROP_EDITABLE
};

static GType tomoe_type_dict_xml = 0;
static GObjectClass *parent_class;

static GObject     *constructor               (GType                  type,
                                               guint                  n_props,
                                               GObjectConstructParam *props);
static void         dispose                   (GObject       *object);
static void         set_property              (GObject       *object,
                                               guint         prop_id,
                                               const GValue  *value,
                                               GParamSpec    *pspec);
static void         get_property              (GObject       *object,
                                               guint          prop_id,
                                               GValue        *value,
                                               GParamSpec    *pspec);
static const gchar *get_name                  (TomoeDict     *dict);
static gboolean     register_char             (TomoeDict     *dict,
                                               TomoeChar     *chr);
static gboolean     unregister_char           (TomoeDict     *dict,
                                               const gchar   *utf8);
static TomoeChar   *get_char                  (TomoeDict     *dict,
                                               const gchar   *utf8);
static GList       *search                    (TomoeDict     *dict,
                                               TomoeQuery    *query);
static gboolean     flush                     (TomoeDict     *dict);
static gboolean     is_editable               (TomoeDict     *dict);
static gchar       *get_available_private_utf8 (TomoeDict    *dict);
static gboolean     tomoe_dict_xml_load       (TomoeDictXML  *dict);
static gboolean     tomoe_dict_xml_save       (TomoeDictXML  *dict);

GType
tomoe_dict_xml_get_type (void)
{
    return tomoe_type_dict_xml;
}

static void
class_init (TomoeDictXMLClass *klass)
{
    GObjectClass *gobject_class;
    TomoeDictClass *dict_class;

    parent_class = g_type_class_peek_parent (klass);

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructor  = constructor;
    gobject_class->dispose      = dispose;
    gobject_class->set_property = set_property;
    gobject_class->get_property = get_property;

    dict_class = TOMOE_DICT_CLASS (klass);
    dict_class->get_name        = get_name;
    dict_class->register_char   = register_char;
    dict_class->unregister_char = unregister_char;
    dict_class->get_char        = get_char;
    dict_class->search          = search;
    dict_class->flush           = flush;
    dict_class->is_editable     = is_editable;
    dict_class->get_available_private_utf8 = get_available_private_utf8;


    g_object_class_install_property (
        gobject_class,
        PROP_FILENAME,
        g_param_spec_string (
            "filename",
            "Filename",
            "The filename of xml file",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property(
        gobject_class,
        PROP_EDITABLE,
        g_param_spec_boolean(
            "editable",
            "Editable",
            "Editable flag",
            TRUE,
            G_PARAM_READWRITE));

    g_type_class_add_private (gobject_class, sizeof (TomoeDictXMLPrivate));
}

static void
init (TomoeDictXML *dict)
{
    TomoeDictXMLPrivate *priv = TOMOE_DICT_XML_GET_PRIVATE (dict);

    priv->filename = NULL;
    priv->name     = NULL;
    priv->chars    = g_ptr_array_new();
    priv->modified = FALSE;
    priv->editable = FALSE;
}

static void
register_type (GTypeModule *type_module)
{
    static const GTypeInfo info =
        {
            sizeof (TomoeDictXMLClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (TomoeDictXML),
            0,
            (GInstanceInitFunc) init,
        };

    tomoe_type_dict_xml = g_type_module_register_type (type_module,
                                                       TOMOE_TYPE_DICT,
                                                       "TomoeDictXML",
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
    return g_object_new_valist (TOMOE_TYPE_DICT_XML, first_property, args);
}

static GObject *
constructor (GType type, guint n_props,
             GObjectConstructParam *props)
{
    GObject *object;
    GObjectClass *klass = G_OBJECT_CLASS (parent_class);
    TomoeDictXML *dict;

    object = klass->constructor (type, n_props, props);
    dict = TOMOE_DICT_XML (object);

    tomoe_dict_xml_load (dict);

    return object;
}

static void
set_property (GObject *object,
              guint prop_id,
              const GValue *value,
              GParamSpec *pspec)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (object);
    TomoeDictXMLPrivate *priv;

    priv =  TOMOE_DICT_XML_GET_PRIVATE (dict);
    switch (prop_id) {
      case PROP_FILENAME:
        priv->filename = g_value_dup_string (value);
        break;
      case PROP_EDITABLE:
        priv->editable = g_value_get_boolean (value);
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
    TomoeDictXML *dict = TOMOE_DICT_XML (object);
    TomoeDictXMLPrivate *priv;

    priv =  TOMOE_DICT_XML_GET_PRIVATE (dict);
    switch (prop_id) {
      case PROP_FILENAME:
        g_value_set_string (value, priv->filename);
        break;
      case PROP_EDITABLE:
        g_value_set_boolean (value, priv->editable);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
dispose (GObject *object)
{
    TomoeDictXML *dict;
    TomoeDictXMLPrivate *priv;

    dict = TOMOE_DICT_XML(object);
    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);

    flush (TOMOE_DICT (dict));

    if (priv->name)
        g_free (priv->name);
    if (priv->filename)
        g_free (priv->filename);
    if (priv->chars)
        TOMOE_PTR_ARRAY_FREE_ALL(priv->chars, g_object_unref);

    priv->name     = NULL;
    priv->filename = NULL;
    priv->chars    = NULL;

    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static const gchar*
get_name (TomoeDict *_dict)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    TomoeDictXMLPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), NULL);
    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);
    return priv->name;
}

static gboolean
register_char (TomoeDict *_dict, TomoeChar *chr)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    TomoeDictXMLPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), FALSE);
    g_return_val_if_fail (TOMOE_IS_CHAR (chr), FALSE);

    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);
    if (_tomoe_dict_ptr_array_register_char (priv->chars, chr)) {
        priv->modified = TRUE;
        return TRUE;
    } else {
        return FALSE;
    }
}

static gboolean
unregister_char (TomoeDict *_dict, const gchar *utf8)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    TomoeDictXMLPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), FALSE);

    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);
    if (_tomoe_dict_ptr_array_unregister_char (priv->chars, utf8)) {
        priv->modified = TRUE;
        return TRUE;
    } else {
        return FALSE;
    }
}

static TomoeChar *
get_char (TomoeDict *_dict, const gchar *utf8)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    TomoeDictXMLPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), NULL);

    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);
    return _tomoe_dict_ptr_array_get_char (priv->chars, utf8);
}

static GList *
search (TomoeDict *_dict, TomoeQuery *query)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    TomoeDictXMLPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), FALSE);

    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);
    return _tomoe_dict_ptr_array_search (priv->chars, query);
}

static gboolean
flush (TomoeDict *_dict)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    TomoeDictXMLPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), FALSE);

    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);
    if (priv->editable && priv->modified) {
        priv->modified = FALSE;
        return tomoe_dict_xml_save (dict);
    } else {
        return TRUE;
    }
}

static gboolean
is_editable (TomoeDict *_dict)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    TomoeDictXMLPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), FALSE);

    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);
    return priv->editable;
}

static gchar *
get_available_private_utf8 (TomoeDict *_dict)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    TomoeDictXMLPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), NULL);

    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);
    return _tomoe_dict_ptr_array_get_available_private_utf8 (priv->chars);
}

static gboolean
tomoe_dict_xml_load (TomoeDictXML *dict)
{
    gboolean success = TRUE;
    TomoeXMLParsedData result;
    TomoeDictXMLPrivate *priv;

    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);

    result.name = NULL;
    result.chars = priv->chars;
    success = _tomoe_xml_parser_parse_dictionary_file (priv->filename, &result);
    if (result.name) {
        g_free (priv->name);
        priv->name = g_strdup (result.name);
        g_free (result.name);
    }
    _tomoe_dict_ptr_array_sort (priv->chars);

    return success;
}

static gboolean
tomoe_dict_xml_save (TomoeDictXML *dict)
{
    TomoeDictXMLPrivate *priv;
    FILE *f;
    gchar *head;
    const gchar *foot = "</dictionary>\n";
    guint i;


    g_return_val_if_fail (TOMOE_IS_DICT (dict), FALSE);

    priv = TOMOE_DICT_XML_GET_PRIVATE (dict);
    if (!priv->editable) return FALSE;

    f = fopen (priv->filename, "wb");
    g_return_val_if_fail (f, FALSE);

    /* write the header */
    head = g_strdup (
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
        "<!DOCTYPE dictionary SYSTEM \"tomoe-dict.dtd\">\n");
    if (fwrite (head, strlen (head), 1, f) < 1) goto ERROR;
    g_free (head);

    if (priv->name)
        head = g_markup_printf_escaped ("<dictionary name=\"%s\">\n",
                                        priv->name);
    else
        head = g_strdup ("<dictionary>\n");
    if (fwrite (head, strlen (head), 1, f) < 1) goto ERROR;

    /* write each characters */
    for (i = 0; i < priv->chars->len; i++) {
        gchar *xml;
        gboolean failed;
        TomoeChar* chr = (TomoeChar*)g_ptr_array_index (priv->chars, i);

        xml = tomoe_char_to_xml (chr);
        if (!xml) goto ERROR;

        failed = fwrite (xml, strlen (xml), 1, f) < 1;
        g_free (xml);
        if (failed) goto ERROR;
    }

    /* close root element */
    if (fwrite (foot, strlen (foot), 1, f) < 1) goto ERROR;

    /* clean */
    g_free (head);
    fclose (f);
    priv->modified = FALSE;
    return TRUE;

ERROR:
    g_free (head);
    g_warning ("Faild to write %s.", priv->filename);
    fclose (f);
    return FALSE;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
