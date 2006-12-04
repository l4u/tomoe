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

#include "tomoe-unihan.h"

#define TOMOE_TYPE_DICT_UNIHAN            tomoe_type_dict_unihan
#define TOMOE_DICT_UNIHAN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_DICT_UNIHAN, TomoeDictUnihan))
#define TOMOE_DICT_UNIHAN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_DICT_UNIHAN, TomoeDictUnihanClass))
#define TOMOE_IS_DICT_UNIHAN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_DICT_UNIHAN))
#define TOMOE_IS_DICT_UNIHAN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_DICT_UNIHAN))
#define TOMOE_DICT_UNIHAN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_DICT_UNIHAN, TomoeDictUnihanClass))

enum {
    PROP_0,
    PROP_NAME
};

#define DEFAULT_NAME "Unihan"

typedef struct _TomoeDictUnihan TomoeDictUnihan;
typedef struct _TomoeDictUnihanClass TomoeDictUnihanClass;
struct _TomoeDictUnihan
{
    TomoeDict            object;
    gchar               *name;
};

struct _TomoeDictUnihanClass
{
    TomoeDictClass parent_class;
};

typedef struct _TomoeDictSearchContext {
    TomoeQuery *query;
    GList *results;
} TomoeDictSearchContext;

static GType tomoe_type_dict_unihan = 0;
static GObjectClass *parent_class;
static GPtrArray *chars = NULL;

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
static TomoeChar    *get_char                 (TomoeDict     *dict,
                                               const gchar   *utf8);
static GList        *search                   (TomoeDict     *dict,
                                               TomoeQuery    *query);
static gboolean      flush                    (TomoeDict     *dict);

static void
class_init (TomoeDictUnihanClass *klass)
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

    g_object_class_install_property (
        gobject_class,
        PROP_NAME,
        g_param_spec_string (
            "name",
            "Name",
            "The name of the dictionary",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
init (TomoeDictUnihan *dict)
{
    dict->name     = NULL;
}

static void
register_type (GTypeModule *type_module)
{
    static const GTypeInfo info =
        {
            sizeof (TomoeDictUnihanClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (TomoeDictUnihan),
            0,
            (GInstanceInitFunc) init,
        };

    tomoe_type_dict_unihan = g_type_module_register_type (type_module,
                                                       TOMOE_TYPE_DICT,
                                                       "TomoeDictUnihan",
                                                       &info, 0);
}

G_MODULE_EXPORT void
TOMOE_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    register_type (type_module);

    if (!chars)
        chars = _tomoe_unihan_create ();
}

G_MODULE_EXPORT void
TOMOE_MODULE_IMPL_EXIT (void)
{
    if (chars) {
        TOMOE_PTR_ARRAY_FREE_ALL (chars, g_object_unref);
        chars = NULL;
    }
}

G_MODULE_EXPORT GObject *
TOMOE_MODULE_IMPL_INSTANTIATE (const gchar *first_property, va_list args)
{
    return g_object_new_valist (TOMOE_TYPE_DICT_UNIHAN, first_property, args);
}

static GObject *
constructor (GType type, guint n_props,
             GObjectConstructParam *props)
{
    GObject *object;
    GObjectClass *klass = G_OBJECT_CLASS (parent_class);

    object = klass->constructor (type, n_props, props);

    return object;
}

static void
set_property (GObject *object,
              guint prop_id,
              const GValue *value,
              GParamSpec *pspec)
{
    TomoeDictUnihan *dict = TOMOE_DICT_UNIHAN (object);

    switch (prop_id) {
      case PROP_NAME:
        g_free (dict->name);
        dict->name = g_value_dup_string (value);
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
    TomoeDict *_dict = TOMOE_DICT (object);

    switch (prop_id) {
      case PROP_NAME:
        g_value_set_string (value, get_name (_dict));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
dispose (GObject *object)
{
    TomoeDictUnihan *dict;

    dict = TOMOE_DICT_UNIHAN (object);

    if (dict->name)
        g_free (dict->name);

    dict->name     = NULL;

    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static const gchar*
get_name (TomoeDict *_dict)
{
    TomoeDictUnihan *dict = TOMOE_DICT_UNIHAN (_dict);
    g_return_val_if_fail (TOMOE_IS_DICT_UNIHAN (dict), NULL);
    return dict->name ? dict->name : DEFAULT_NAME;
}

static gboolean
register_char (TomoeDict *_dict, TomoeChar *chr)
{
    TomoeDictUnihan *dict = TOMOE_DICT_UNIHAN (_dict);

    g_return_val_if_fail (TOMOE_IS_DICT_UNIHAN (dict), FALSE);
    g_return_val_if_fail (chr, FALSE);

    g_warning ("TomoeDictUnihan doesn't support register_char()");
    return FALSE;
}

static gboolean
unregister_char (TomoeDict *_dict, const gchar *utf8)
{
    TomoeDictUnihan *dict = TOMOE_DICT_UNIHAN (_dict);

    g_return_val_if_fail (TOMOE_IS_DICT_UNIHAN (dict), FALSE);
    g_return_val_if_fail (utf8 && *utf8 != '\0', FALSE);

    g_warning ("TomoeDictUnihan doesn't support register_char()");
    return FALSE;
}

static TomoeChar *
get_char (TomoeDict *_dict, const gchar *utf8)
{
    TomoeDictUnihan *dict = TOMOE_DICT_UNIHAN (_dict);
    guint i;

    g_return_val_if_fail (TOMOE_IS_DICT_UNIHAN (dict), NULL);
    g_return_val_if_fail (utf8 && *utf8 != '\0', NULL);

    for (i = 0; i < chars->len; i++) {
        TomoeChar *chr = g_ptr_array_index (chars, i);
        if (0 == strcmp(tomoe_char_get_utf8(chr), utf8)) {
            return chr;
        }
    }

    return NULL;
}

static gboolean
tomoe_dict_unihan_does_match_char_with_n_strokes (TomoeChar *chr,
                                                  gint min, gint max)
{
    TomoeWriting *writing;
    gint n_strokes;

    if (min < 0 && max < 0)
        return TRUE;

    writing = tomoe_char_get_writing (chr);
    if (!writing)
        return FALSE;

    n_strokes = tomoe_writing_get_n_strokes (writing);
    return ((min < 0 || min <= n_strokes) &&
            (max < 0 || max >= n_strokes));
}

static gint
tomoe_dict_unihan_compare_reading (gconstpointer a, gconstpointer b)
{
    TomoeReading *reading, *searched_reading;

    reading = TOMOE_READING(a);
    searched_reading = TOMOE_READING(b);
    return strcmp(tomoe_reading_get_reading(reading),
                  tomoe_reading_get_reading(searched_reading));
}

static gboolean
tomoe_dict_unihan_does_match_char_with_readings (TomoeChar *chr,
                                                 TomoeReading *reading)
{
    if (!reading)
        return TRUE;

    if (g_list_find_custom ((GList *)tomoe_char_get_readings (chr),
                            reading, tomoe_dict_unihan_compare_reading))
        return TRUE;
    else
        return FALSE;
}

static void
tomoe_dict_unihan_collect_chars_by_query (gpointer data, gpointer user_data)
{
    TomoeChar *chr = data;
    TomoeDictSearchContext *context = user_data;
    TomoeQuery *query;
    TomoeReading *reading;
    gint min_n_strokes, max_n_strokes;

    query = context->query;

    min_n_strokes = tomoe_query_get_min_n_strokes (query);
    max_n_strokes = tomoe_query_get_max_n_strokes (query);
    if (!tomoe_dict_unihan_does_match_char_with_n_strokes (chr,
                                                        min_n_strokes,
                                                        max_n_strokes))
        return;

    reading = g_list_nth_data ((GList *)tomoe_query_get_readings (query), 0);
    if (!tomoe_dict_unihan_does_match_char_with_readings (chr, reading))
        return;

    context->results = g_list_prepend (context->results,
                                       tomoe_candidate_new (chr));
}

static GList *
search (TomoeDict *_dict, TomoeQuery *query)
{
    TomoeDictSearchContext search_context;

    search_context.query = g_object_ref (query);
    search_context.results = NULL;

    g_ptr_array_foreach_reverse (chars,
                                 tomoe_dict_unihan_collect_chars_by_query,
                                 &search_context);
    g_object_unref (search_context.query);

    return search_context.results;
}

static gboolean
flush (TomoeDict *_dict)
{
    return TRUE;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
