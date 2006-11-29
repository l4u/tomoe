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

#include "tomoe-dict-impl.h"
#include "tomoe-candidate.h"
#include "glib-utils.h"

#define TOMOE_TYPE_DICT_XML            tomoe_type_dict_xml
#define TOMOE_DICT_XML(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_DICT_XML, TomoeDictXML))
#define TOMOE_DICT_XML_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_DICT_XML, TomoeDictXMLClass))
#define TOMOE_IS_DICT_XML(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_DICT_XML))
#define TOMOE_IS_DICT_XML_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_DICT_XML))
#define TOMOE_DICT_XML_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_DICT_XML, TomoeDictXMLClass))

enum {
    PROP_0,
    PROP_FILENAME,
    PROP_EDITABLE
};

typedef struct _TomoeDictXML TomoeDictXML;
typedef struct _TomoeDictXMLClass TomoeDictXMLClass;
struct _TomoeDictXML
{
    TomoeDict            object;
    gchar               *filename;
    gchar               *name;
    GPtrArray           *chars;

    gboolean             editable;
    gboolean             modified;
};

struct _TomoeDictXMLClass
{
    TomoeDictClass parent_class;
};

typedef struct _TomoeDictSearchContext {
    TomoeQuery *query;
    GList *results;
} TomoeDictSearchContext;

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
static TomoeChar    *get_char                 (TomoeDict     *dict,
                                               const gchar   *utf8);
static GList        *search                   (TomoeDict     *dict,
                                               TomoeQuery    *query);
static gboolean      tomoe_dict_xml_load      (TomoeDictXML  *dict);
static void          tomoe_dict_xml_save      (TomoeDictXML  *dict);
static gint          letter_compare_func      (gconstpointer  a,
                                               gconstpointer  b);

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
}

static void
init (TomoeDictXML *dict)
{
    dict->filename = NULL;
    dict->name     = NULL;
    dict->chars    = g_ptr_array_new();
    dict->modified = FALSE;
    dict->editable = FALSE;
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
TOMOE_DICT_IMPL_INIT (GTypeModule *type_module)
{
    register_type (type_module);
}

G_MODULE_EXPORT void
TOMOE_DICT_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
TOMOE_DICT_IMPL_INSTANTIATE (const gchar *first_property, va_list args)
{
    GObject *object;

    object = g_object_new_valist (TOMOE_TYPE_DICT_XML, 
                                  first_property, args);

    return object;
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

    switch (prop_id) {
    case PROP_FILENAME:
        dict->filename = g_value_dup_string (value);
        break;
    case PROP_EDITABLE:
        dict->editable = g_value_get_boolean (value);
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

    switch (prop_id) {
    case PROP_FILENAME:
        g_value_set_string (value, dict->filename);
        break;
    case PROP_EDITABLE:
        g_value_set_boolean (value, dict->editable);
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

    dict = TOMOE_DICT_XML(object);

    if (dict->editable && dict->modified) {
        dict->modified = FALSE;
        tomoe_dict_xml_save (dict);
    }

    if (dict->name)
        g_free (dict->name);
    if (dict->filename)
        g_free (dict->filename);
    if (dict->chars)
        TOMOE_PTR_ARRAY_FREE_ALL(dict->chars, g_object_unref);

    dict->name     = NULL;
    dict->filename = NULL;
    dict->chars    = NULL;

    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static const gchar*
get_name (TomoeDict *_dict)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), NULL);
    return dict->name;
}

static gboolean
register_char (TomoeDict *_dict, TomoeChar *add)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), FALSE);
    g_return_val_if_fail (add, FALSE);

    g_ptr_array_add (dict->chars, g_object_ref (G_OBJECT (add)));
    g_ptr_array_sort (dict->chars, letter_compare_func);
    dict->modified = TRUE;

    return TRUE;
}

static gboolean
unregister_char (TomoeDict *_dict, const gchar *utf8)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    GPtrArray *chars;
    TomoeChar *removed = NULL;
    guint i, index = -1;

    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), FALSE);
    g_return_val_if_fail (utf8 && *utf8 != '\0', FALSE);

    chars = dict->chars;
    for (i = 0; i < chars->len; i++) {
        TomoeChar *chr = g_ptr_array_index (chars, i);
        if (0 == strcmp(tomoe_char_get_utf8(chr), utf8)) {
            index = i;
            removed = chr;
            break;
        }
    }

    if (index >= 0) {
        g_ptr_array_remove_index (chars, index);
        g_object_unref (removed);
        dict->modified = TRUE;
        return TRUE;
    } else {
        return FALSE;
    }
}

static TomoeChar *
get_char (TomoeDict *_dict, const gchar *utf8)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    GPtrArray *chars;
    guint i;

    g_return_val_if_fail (TOMOE_IS_DICT_XML (dict), NULL);
    g_return_val_if_fail (utf8 && *utf8 != '\0', NULL);

    chars = dict->chars;
    for (i = 0; i < chars->len; i++) {
        TomoeChar *chr = g_ptr_array_index (chars, i);
        if (0 == strcmp(tomoe_char_get_utf8(chr), utf8)) {
            return chr;
        }
    }

    return NULL;
}

static gboolean
tomoe_dict_xml_does_match_char_with_n_strokes (TomoeChar *chr,
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
tomoe_dict_xml_compare_reading (gconstpointer a, gconstpointer b)
{
    TomoeReading *reading, *searched_reading;

    reading = TOMOE_READING(a);
    searched_reading = TOMOE_READING(b);
    return strcmp(tomoe_reading_get_reading(reading),
                  tomoe_reading_get_reading(searched_reading));
}

static gboolean
tomoe_dict_xml_does_match_char_with_readings (TomoeChar *chr,
                                              TomoeReading *reading)
{
    if (!reading)
        return TRUE;

    if (g_list_find_custom ((GList *)tomoe_char_get_readings (chr),
                            reading, tomoe_dict_xml_compare_reading))
        return TRUE;
    else
        return FALSE;
}

static void
tomoe_dict_xml_collect_chars_by_query (gpointer data, gpointer user_data)
{
    TomoeChar *chr = data;
    TomoeDictSearchContext *context = user_data;
    TomoeQuery *query;
    TomoeReading *reading;
    gint min_n_strokes, max_n_strokes;

    query = context->query;

    min_n_strokes = tomoe_query_get_min_n_strokes (query);
    max_n_strokes = tomoe_query_get_max_n_strokes (query);
    if (!tomoe_dict_xml_does_match_char_with_n_strokes (chr,
                                                        min_n_strokes,
                                                        max_n_strokes))
        return;

    reading = g_list_nth_data ((GList *)tomoe_query_get_readings (query), 0);
    if (!tomoe_dict_xml_does_match_char_with_readings (chr, reading))
        return;

    context->results = g_list_prepend (context->results,
                                       tomoe_candidate_new (chr));
}

static GList *
search (TomoeDict *_dict, TomoeQuery *query)
{
    TomoeDictXML *dict = TOMOE_DICT_XML (_dict);
    TomoeDictSearchContext search_context;

    search_context.query = g_object_ref (query);
    search_context.results = NULL;

    g_ptr_array_foreach_reverse (dict->chars,
                                 tomoe_dict_xml_collect_chars_by_query,
                                 &search_context);
    g_object_unref (search_context.query);

    return search_context.results;
}

static gint
letter_compare_func (gconstpointer a, gconstpointer b)
{
    TomoeChar *ca = *(TomoeChar **) a;
    TomoeChar *cb = *(TomoeChar **) b;
    return tomoe_char_compare (ca, cb);
}



/*
 *  XML related functions and data types.
 */
typedef enum {
    STATE_NONE,
    STATE_UTF8,
    STATE_N_STROKES,
    STATE_STROKES,
    STATE_READINGS,
    STATE_READING,
    STATE_WRITING,
    STATE_STROKE,
    STATE_POINT,
    STATE_META
} TomoeCharState;

typedef struct _ParseData
{
    TomoeDictXML     *dict;

    gboolean          in_dict;
    TomoeCharState    state;

    TomoeChar        *chr;
    TomoeWriting     *writing;
    gint              n_points;
    TomoeReadingType  reading_type;

    /* meta data */
    gchar *key;
    gchar *value;
} ParseData;

static void
set_parse_error (GMarkupParseContext *context, GError **error,
                 ParseData *data)
{
    gchar buf[1024];
    gint line = 0, chr = 0;

    if (!error) return;

    g_markup_parse_context_get_position (context, &line, &chr);

    g_snprintf (buf, G_N_ELEMENTS (buf),
                "Invalid content at line %d char %d of %s.",
                line, chr, data->dict->filename);

    *error = g_error_new (G_MARKUP_ERROR,
                          G_MARKUP_ERROR_INVALID_CONTENT,
                          buf);
}

static void
start_element_handler (GMarkupParseContext *context,
                       const gchar         *element_name,
                       const gchar        **attr_names,
                       const gchar        **attr_values,
                       gpointer             user_data,
                       GError             **error)
{
    ParseData *data = user_data;

    if (!strcmp ("dictionary", element_name)) {
        gint idx;

        for (idx = 0; attr_names && attr_names[idx]; idx++) {
            if (!strcmp ("name", attr_names[idx])) {
                g_free (data->dict->name);
                data->dict->name = g_strdup (attr_values[idx]);
            }
        }
        data->in_dict = TRUE;
        return;
    }

    if (!data->in_dict) {
        set_parse_error (context, error, data);
        return;
    }

    if (!strcmp ("character", element_name)) {
        data->chr = tomoe_char_new ();
        return;
    }

    if (!data->chr) {
        set_parse_error (context, error, data);
        return;
    }

    if (!strcmp ("utf8", element_name)) {
        data->state = STATE_UTF8;
        return;
    }

    if (!strcmp ("number-of-strokes", element_name)) {
        data->state = STATE_N_STROKES;
        return;
    }

    if (!strcmp ("strokes", element_name)) {
        data->state = STATE_WRITING;
        data->writing = tomoe_writing_new ();
        return;
    }

    if (!strcmp ("stroke", element_name)) {
        if (!data->writing) {
            set_parse_error (context, error, data);
            return;
        }
        data->state = STATE_STROKE;
        data->n_points = 0;
        return;
    }

    if (!strcmp ("point", element_name)) {
        gint idx, x = -1, y = -1;

        if (data->state != STATE_STROKE) {
            set_parse_error (context, error, data);
            return;
        }

        data->state = STATE_POINT;

        for (idx = 0; attr_names && attr_names[idx]; idx++) {
            if (!strcmp ("x", attr_names[idx])) {
                x = atoi (attr_values[idx]);
            } else if (!strcmp ("y", attr_names[idx])) {
                y = atoi (attr_values[idx]);
            }
        }

        if (x < 0 || y < 0 || x >= 1000 || y >= 1000) {
            g_warning ("Invalid writing data: %s: x = %d, y = %d\n",
                       tomoe_char_get_utf8 (data->chr), x, y);
        }

        if (data->n_points == 0)
            tomoe_writing_move_to (data->writing, x, y);
        else
            tomoe_writing_line_to (data->writing, x, y);

        data->n_points++;
        return;
    }

    if (!strcmp ("readings", element_name)) {
        data->state = STATE_READINGS;
        return;
    }

    if (!strcmp ("reading", element_name)) {
        gint idx;

        if (data->state != STATE_READINGS) {
            set_parse_error (context, error, data);
            return;
        }

        data->state = STATE_READING;
        data->reading_type = TOMOE_READING_UNKNOWN;

        for (idx = 0; attr_names && attr_names[idx]; idx++) {
            if (!strcmp ("type", attr_names[idx])) {
                if (!strcmp ("ja_on", attr_values[idx]))
                    data->reading_type = TOMOE_READING_JA_ON;
                else if (!strcmp ("ja_kun", attr_values[idx]))
                    data->reading_type = TOMOE_READING_JA_KUN;
            }
        }

        return;
    }

    if (!strcmp ("meta", element_name)) {
        data->state = STATE_META;
        return;
    }

    if (data->state == STATE_META) {
        g_free (data->key);
        g_free (data->value);
        data->key   = g_strdup (element_name);
        data->value = NULL;
    }

    /* throw error? */
}

static void
end_element_handler (GMarkupParseContext *context,
                     const gchar         *element_name,
                     gpointer             user_data,
                     GError             **error)
{
    ParseData *data = user_data;

    if (!strcmp ("dictionary", element_name)) {
        data->in_dict = FALSE;
        return;
    }

    if (!strcmp ("character", element_name)) {
        if (tomoe_char_get_utf8 (data->chr))
            g_ptr_array_add (data->dict->chars, data->chr);
        else
            g_object_unref (G_OBJECT (data->chr));
        data->chr = NULL;
        return;
    }

    if (!strcmp("utf8", element_name)) {
        data->state = STATE_NONE;
        return;
    }

    if (!strcmp ("number-of-strokes", element_name)) {
        data->state = STATE_NONE;
        return;
    }

    if (!strcmp ("strokes", element_name)) {
        if (data->chr && data->writing)
            tomoe_char_set_writing (data->chr, data->writing);
        data->writing = NULL;
        data->state = STATE_NONE;
        return;
    }

    if (!strcmp ("stroke", element_name)) {
        data->state = STATE_WRITING;
        data->n_points = 0;
        return;
    }

    if (!strcmp ("point", element_name)) {
        data->state = STATE_STROKE;
        return;
    }

    if (!strcmp ("readings", element_name)) {
        data->state = STATE_NONE;
        data->reading_type = TOMOE_READING_INVALID;
        return;
    }

    if (!strcmp ("reading", element_name)) {
        data->state = STATE_READINGS;
        return;
    }

    if (!strcmp ("meta", element_name)) {
        data->state = STATE_NONE;
        return;
    }

    if (data->state == STATE_META) {
        if (data->chr && data->key && data->value)
            tomoe_char_register_meta_data (data->chr, data->key, data->value);
        g_free (data->key);
        g_free (data->value);
        data->key   = NULL;
        data->value = NULL;
    }
}

static void
text_handler (GMarkupParseContext *context,
              const gchar         *text,
              gsize                text_len,  
              gpointer             user_data,
              GError             **error)
{
    ParseData *data = user_data;

    switch (data->state) {
    case STATE_UTF8:
    {
        tomoe_char_set_utf8 (data->chr, text);
        return;
    }
    case STATE_N_STROKES:
    {
        tomoe_char_set_n_strokes (data->chr, atoi (text));
        return;
    }
    case STATE_READING:
    {
        TomoeReading *reading;

        reading = tomoe_reading_new (data->reading_type, text);
        tomoe_char_add_reading (data->chr, reading);
        g_object_unref (reading);
        return;
    }
    case STATE_META:
    {
        g_free (data->value);
        data->value = g_strdup (text);
        return;
    }
    default:
        break;
    }
}

static void
passthrough_handler (GMarkupParseContext *context,
                     const gchar         *text,
                     gsize                text_len,  
                     gpointer             user_data,
                     GError             **error)
{
}

static void
error_handler (GMarkupParseContext *context,
               GError              *error,
               gpointer             user_data)
{
}

static GMarkupParser parser = {
    start_element_handler,
    end_element_handler,
    text_handler,
    passthrough_handler,
    error_handler,
};

static gboolean
tomoe_dict_xml_load (TomoeDictXML *dict)
{
    GMarkupParseContext *context;
    FILE *f;
    gint bytes;
    gchar buf[4096];
    ParseData data;
    gboolean retval = TRUE;

    f = fopen (dict->filename, "rb");
    g_return_val_if_fail (f, FALSE);

    data.dict         = dict;
    data.in_dict      = FALSE;
    data.state        = STATE_NONE;
    data.chr          = NULL;
    data.writing      = NULL;
    data.key          = NULL;
    data.value        = NULL;
    data.n_points     = 0;
    data.reading_type = TOMOE_READING_INVALID;

    context = g_markup_parse_context_new (&parser, 0, &data, NULL);

    while ((bytes = fread (buf, sizeof (gchar), 4096, f)) > 0) {
        GError *error = NULL;
        gboolean success;

        success = g_markup_parse_context_parse(context, buf, bytes, &error);
        if (!success) {
            g_warning("Tomoe XML Dictionary: %s", error->message);
            g_error_free(error);
            retval = FALSE;
            break;
        }
    }

    fclose (f);
    g_markup_parse_context_free (context);

    if (dict->chars)
        g_ptr_array_sort (dict->chars, letter_compare_func);

    return retval;
}

static void
tomoe_dict_xml_save (TomoeDictXML *dict)
{
    FILE *f;
    gchar *head;
    const gchar *foot = "</dictionary>\n";
    guint i;

    g_return_if_fail (TOMOE_IS_DICT (dict));
    if (!dict->editable) return;

    f = fopen (dict->filename, "wb");
    g_return_if_fail (f);

    /* write the header */
    head = g_strdup (
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
        "<!DOCTYPE dictionary SYSTEM \"tomoe-dict.dtd\">\n");
    if (fwrite (head, strlen (head), 1, f) < 1) goto ERROR;
    g_free (head);

    if (dict->name)
        head = g_markup_printf_escaped ("<dictionary name=\"%s\">\n",
                                        dict->name);
    else
        head = g_strdup ("<dictionary>\n");
    if (fwrite (head, strlen (head), 1, f) < 1) goto ERROR;

    /* write each characters */
    for (i = 0; i < dict->chars->len; i++) {
        gchar *xml;
        gboolean failed;
        TomoeChar* chr = (TomoeChar*)g_ptr_array_index (dict->chars, i);

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
    dict->modified = FALSE;
    return;

ERROR:
    g_free (head);
    g_warning ("Faild to write %s.", dict->filename);
    fclose (f);
    return;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
