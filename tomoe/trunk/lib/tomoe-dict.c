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

#include "tomoe-dict.h"
#include "tomoe-char.h"
#include "tomoe-candidate.h"
#include "glib-utils.h"

#define LIMIT_LENGTH ((300 * 0.25) * (300 * 0.25))
#define LINE_BUF_SIZE 4096
#define DICT_LETTER_INITIAL_SIZE 3049
#define DICT_LETTER_EXPAND_SIZE 10

#define TOMOE_DICT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_DICT, TomoeDictPrivate))

typedef struct _TomoeDictPrivate TomoeDictPrivate;
struct _TomoeDictPrivate
{
    char                *filename;
    char                *name;
    GPtrArray           *chars;

    gboolean             editable;
    gboolean             modified;
};

typedef struct _TomoeDictSearchContext {
    gint min_n_strokes;
    gint max_n_strokes;
    TomoeReading *reading;
    GList *results;
} TomoeDictSearchContext;


enum
{
  PROP_0,
  PROP_NAME,
  PROP_FILENAME,
  PROP_EDITABLE,
  PROP_MODIFIED,
};

G_DEFINE_TYPE (TomoeDict, tomoe_dict, G_TYPE_OBJECT)

static void     tomoe_dict_dispose        (GObject         *object);
static void     tomoe_dict_set_property   (GObject         *object,
                                           guint            prop_id,
                                           const GValue    *value,
                                           GParamSpec      *pspec);
static void     tomoe_dict_get_property   (GObject         *object,
                                           guint            prop_id,
                                           GValue          *value,
                                           GParamSpec       *pspec);
static gboolean tomoe_dict_load           (TomoeDict       *dict);
static gint     letter_compare_func       (gconstpointer    a,
                                           gconstpointer    b);
#if 1
/*
 *  XML related functions.
 *  These functions will be moved to external class.
 */
static gboolean tomoe_dict_load_xml       (TomoeDict       *dict);
static void     tomoe_dict_save_xml       (TomoeDict       *dict);
#endif


static void
tomoe_dict_class_init (TomoeDictClass *klass)
{
    GObjectClass *gobject_class;
    GParamSpec *spec;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = tomoe_dict_dispose;
    gobject_class->set_property = tomoe_dict_set_property;
    gobject_class->get_property = tomoe_dict_get_property;

    spec = g_param_spec_string ("name",
                                N_("Name"),
                                N_("A name of the dictionary."),
                                NULL,
                                G_PARAM_READABLE);
    g_object_class_install_property (gobject_class, PROP_NAME, spec);

    spec = g_param_spec_string ("filename",
                                N_("Filename"),
                                N_("A filename of the dictionary."),
                                NULL,
                                G_PARAM_READABLE | G_PARAM_WRITABLE |
                                G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_property (gobject_class, PROP_FILENAME, spec);

    spec = g_param_spec_boolean ("editable",
                                 N_("Editable"),
                                 N_("Whether the dictionary is editable."),
                                 FALSE,
                                 G_PARAM_READABLE | G_PARAM_WRITABLE |
                                 G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_property (gobject_class, PROP_EDITABLE, spec);

    spec = g_param_spec_boolean ("modified",
                                 N_("Modified"),
                                 N_("Whether the dictionary is modified."),
                                 FALSE,
                                 G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class, PROP_MODIFIED, spec);

    g_type_class_add_private (gobject_class, sizeof (TomoeDictPrivate));
}

static void
tomoe_dict_init (TomoeDict *dict)
{
    TomoeDictPrivate *priv = TOMOE_DICT_GET_PRIVATE(dict);

    priv->filename = NULL;
    priv->name     = NULL;
    priv->chars    = g_ptr_array_new();
    priv->modified = FALSE;
}


TomoeDict *
tomoe_dict_new (const char* filename, gboolean editable)
{
    TomoeDict *dict;

    if (!filename && !*filename) return NULL;

    dict = g_object_new(TOMOE_TYPE_DICT,
                        "filename", filename,
                        "editable", editable,
                        NULL);

    tomoe_dict_load (dict);

    return dict;
}

static void
tomoe_dict_dispose (GObject *object)
{
    TomoeDict *dict;
    TomoeDictPrivate *priv;

    dict = TOMOE_DICT(object);
    priv = TOMOE_DICT_GET_PRIVATE(dict);

    if (priv->editable && priv->modified) {
        priv->modified = FALSE;
        tomoe_dict_save_xml (dict);
    }

    if (priv->name)
        g_free(priv->name);
    if (priv->filename)
        g_free(priv->filename);
    if (priv->chars)
        TOMOE_PTR_ARRAY_FREE_ALL(priv->chars, g_object_unref);

    priv->name     = NULL;
    priv->filename = NULL;
    priv->chars    = NULL;

    G_OBJECT_CLASS (tomoe_dict_parent_class)->dispose (object);
}

static void
tomoe_dict_set_property (GObject         *object,
                         guint            prop_id,
                         const GValue    *value,
                         GParamSpec      *pspec)
{
    TomoeDictPrivate *priv;

    priv = TOMOE_DICT_GET_PRIVATE(object);

    switch (prop_id) {
      case PROP_FILENAME:
        g_free(priv->filename);
        priv->filename = g_value_dup_string(value);
        break;
      case PROP_EDITABLE:
        priv->editable = g_value_get_boolean(value);
        break;
      case PROP_MODIFIED:
        priv->modified = g_value_get_boolean(value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
tomoe_dict_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    TomoeDictPrivate *priv;

    priv = TOMOE_DICT_GET_PRIVATE(object);

    switch (prop_id) {
      case PROP_NAME:
        g_value_set_string(value, priv->name);
        break;
      case PROP_FILENAME:
        g_value_set_string(value, priv->filename);
        break;
      case PROP_EDITABLE:
        g_value_set_boolean(value, priv->editable);
        break;
      case PROP_MODIFIED:
        g_value_set_boolean(value, priv->modified);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static gboolean
tomoe_dict_load (TomoeDict *dict)
{
    return tomoe_dict_load_xml (dict);
}

const char*
tomoe_dict_get_filename (TomoeDict* dict)
{
    g_return_val_if_fail(dict, NULL);
    return TOMOE_DICT_GET_PRIVATE(dict)->filename;
}

const char*
tomoe_dict_get_name (TomoeDict* dict)
{
    g_return_val_if_fail(dict, NULL);
    return TOMOE_DICT_GET_PRIVATE(dict)->name;
}

gboolean
tomoe_dict_is_editable (TomoeDict* dict)
{
    g_return_val_if_fail(dict, FALSE);
    return TOMOE_DICT_GET_PRIVATE(dict)->editable;
}

gboolean
tomoe_dict_is_modified (TomoeDict *dict)
{
    g_return_val_if_fail(dict, FALSE);
    return TOMOE_DICT_GET_PRIVATE(dict)->modified;
}

void
tomoe_dict_set_modified (TomoeDict *dict, gboolean modified)
{
    g_return_if_fail(dict);
    TOMOE_DICT_GET_PRIVATE(dict)->modified = modified;
    g_object_notify (G_OBJECT (dict), "modified");
}

guint
tomoe_dict_get_size (TomoeDict* dict)
{
    g_return_val_if_fail(dict, 0);
    return TOMOE_DICT_GET_PRIVATE(dict)->chars->len;
}

gboolean
tomoe_dict_register_char (TomoeDict* dict, TomoeChar* add)
{
    TomoeDictPrivate *priv;

    g_return_val_if_fail(dict, FALSE);
    g_return_val_if_fail(add, FALSE);

    priv = TOMOE_DICT_GET_PRIVATE(dict);
    g_ptr_array_add (priv->chars, g_object_ref (G_OBJECT (add)));
    g_ptr_array_sort (priv->chars, letter_compare_func);
    tomoe_dict_set_modified (dict, TRUE);

    return TRUE;
}

gboolean
tomoe_dict_unregister_char (TomoeDict* dict, const gchar *utf8)
{
    GPtrArray *chars;
    TomoeChar *removed = NULL;
    guint i, index = -1;

    g_return_val_if_fail(dict, FALSE);
    g_return_val_if_fail(utf8, FALSE);

    chars = TOMOE_DICT_GET_PRIVATE(dict)->chars;
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
        tomoe_dict_set_modified (dict, TRUE);
        return TRUE;
    } else {
        return FALSE;
    }
}

TomoeChar *
tomoe_dict_get_char (TomoeDict* dict, const gchar *utf8)
{
    GPtrArray *chars;
    guint i;

    g_return_val_if_fail(dict, NULL);
    g_return_val_if_fail(utf8, NULL);

    chars = TOMOE_DICT_GET_PRIVATE(dict)->chars;
    for (i = 0; i < chars->len; i++) {
        TomoeChar *chr = g_ptr_array_index (chars, i);
        if (0 == strcmp(tomoe_char_get_utf8(chr), utf8)) {
            return chr;
        }
    }

    return NULL;
}

static void
tomoe_dict_collect_chars_by_n_strokes (gpointer data, gpointer user_data)
{
    TomoeChar *chr = data;
    TomoeDictSearchContext *context = user_data;
    TomoeWriting *writing;
    gint n_strokes;

    writing = tomoe_char_get_writing (chr);
    if (!writing) return;

    n_strokes = tomoe_writing_get_n_strokes (writing);
    if ((context->min_n_strokes < 0 || context->min_n_strokes <= n_strokes) &&
        (context->max_n_strokes < 0 || context->max_n_strokes >= n_strokes)) {
        context->results = g_list_prepend (context->results,
                                           tomoe_candidate_new (chr));
    }
}

GList *
tomoe_dict_search_by_n_strokes (TomoeDict *dict, gint min, gint max)
{
    TomoeDictPrivate *priv;
    TomoeDictSearchContext context;

    context.min_n_strokes = min;
    context.max_n_strokes = max;
    context.results = NULL;

    priv = TOMOE_DICT_GET_PRIVATE(dict);
    g_ptr_array_foreach_reverse (priv->chars,
                                 tomoe_dict_collect_chars_by_n_strokes,
                                 &context);

    return context.results;
}

static gint
tomoe_dict_compare_reading (gconstpointer a, gconstpointer b)
{
    TomoeReading *reading, *searched_reading;

    reading = TOMOE_READING(a);
    searched_reading = TOMOE_READING(b);
    return strcmp(tomoe_reading_get_reading(reading),
                  tomoe_reading_get_reading(searched_reading));
}

static void
tomoe_dict_collect_chars_by_reading (gpointer data, gpointer user_data)
{
    TomoeChar *chr = data;
    TomoeDictSearchContext *context = user_data;

    if (g_list_find_custom ((GList *)tomoe_char_get_readings (chr),
                            context->reading, tomoe_dict_compare_reading))
        context->results = g_list_prepend (context->results,
                                           tomoe_candidate_new (chr));
}

GList *
tomoe_dict_search_by_reading (TomoeDict* dict, TomoeReading *reading)
{
    TomoeDictPrivate *priv;
    TomoeDictSearchContext context;

    context.reading = reading;
    context.results = NULL;

    priv = TOMOE_DICT_GET_PRIVATE(dict);
    g_ptr_array_foreach_reverse (priv->chars,
                                 tomoe_dict_collect_chars_by_reading,
                                 &context);

    return context.results;
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
 *  These will be moved to external class.
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
    TomoeDict        *dict;
    TomoeDictPrivate *priv;

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
                line, chr, data->priv->filename);

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
                g_free (data->priv->name);
                data->priv->name = g_strdup (attr_values[idx]);
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
        data->reading_type = TOMOE_READING_INVALID;

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

    /* thow error? */
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
            g_ptr_array_add (data->priv->chars, data->chr);
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
tomoe_dict_load_xml (TomoeDict *dict)
{
    TomoeDictPrivate *priv = TOMOE_DICT_GET_PRIVATE(dict);
    GMarkupParseContext *context;
    FILE *f;
    gint bytes;
    gchar buf[4096];
    ParseData data;
    gboolean retval = TRUE;

    f = fopen (priv->filename, "rb");
    g_return_val_if_fail (f, FALSE);

    data.dict         = dict;
    data.priv         = priv;
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

    if (priv->chars)
        g_ptr_array_sort (priv->chars, letter_compare_func);

    return retval;
}

static void
tomoe_dict_save_xml (TomoeDict *dict)
{
    TomoeDictPrivate *priv;
    FILE *f;
    gchar *head;
    const gchar *foot = "</dictionary>\n";
    guint i;

    g_return_if_fail (TOMOE_IS_DICT (dict));
    if (!tomoe_dict_is_editable (dict)) return;

    priv = TOMOE_DICT_GET_PRIVATE(dict);

    f = fopen (priv->filename, "wb");
    g_return_if_fail (f);

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
    tomoe_dict_set_modified (dict, FALSE);
    return;

ERROR:
    g_free (head);
    g_warning ("Faild to write %s.", priv->filename);
    fclose (f);
    return;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
