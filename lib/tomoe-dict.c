/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <math.h>
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
    GPtrArray           *letters;

    gboolean             editable;
    gboolean             modified;
};

typedef struct _TomoeDictSearchContext {
    TomoeReading *reading;
    GList *results;
} TomoeDictSearchContext;


enum
{
  PROP_0,
  PROP_NAME,
  PROP_FILENAME,
  PROP_LETTERS,
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

    spec = g_param_spec_pointer ("letters",
                                 N_("Letters"),
                                 N_("Letters of the dictionary."),
                                 G_PARAM_READABLE);
    g_object_class_install_property (gobject_class, PROP_LETTERS, spec);

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
    priv->letters  = g_ptr_array_new();
    priv->modified = FALSE;
}


TomoeDict*
tomoe_dict_new (const char* filename, gboolean editable)
{
    TomoeDict* dict;

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

    g_free(priv->name);
    g_free(priv->filename);
    if (priv->letters)
        TOMOE_PTR_ARRAY_FREE_ALL(priv->letters, g_object_unref);

    priv->name     = NULL;
    priv->filename = NULL;
    priv->letters  = NULL;

    G_OBJECT_CLASS (tomoe_dict_parent_class)->dispose (object);
}

static void
tomoe_dict_set_property (GObject         *object,
                         guint            prop_id,
                         const GValue    *value,
                         GParamSpec      *pspec)
{
    TomoeDict *dict;
    TomoeDictPrivate *priv;

    dict = TOMOE_DICT(object);
    priv = TOMOE_DICT_GET_PRIVATE(dict);

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
    TomoeDict *dict;
    TomoeDictPrivate *priv;

    dict = TOMOE_DICT(object);
    priv = TOMOE_DICT_GET_PRIVATE(dict);

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

void
tomoe_dict_save (TomoeDict *dict)
{
    tomoe_dict_save_xml (dict);
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
    return TOMOE_DICT_GET_PRIVATE(dict)->letters->len;
}

gboolean
tomoe_dict_register_char (TomoeDict* dict, TomoeChar* add)
{
    TomoeDictPrivate *priv;

    g_return_val_if_fail(dict, FALSE);
    g_return_val_if_fail(add, FALSE);

    priv = TOMOE_DICT_GET_PRIVATE(dict);
    g_ptr_array_add (priv->letters, g_object_ref (G_OBJECT (add)));
    g_ptr_array_sort (priv->letters, letter_compare_func);
    tomoe_dict_set_modified (dict, TRUE);

    return TRUE;
}

gboolean
tomoe_dict_unregister_char (TomoeDict* dict, const gchar *code_point)
{
    GPtrArray *chars;
    TomoeChar *removed = NULL;
    guint i, index = -1;

    g_return_val_if_fail(dict, FALSE);
    g_return_val_if_fail(code_point, FALSE);

    chars = TOMOE_DICT_GET_PRIVATE(dict)->letters;
    for (i = 0; i < chars->len; i++) {
        TomoeChar *chr = g_ptr_array_index (chars, i);
        if (0 == strcmp(tomoe_char_get_code(chr), code_point)) {
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
tomoe_dict_get_char (TomoeDict* dict, const gchar *code_point)
{
    GPtrArray *chars;
    guint i;

    g_return_val_if_fail(dict, NULL);
    g_return_val_if_fail(code_point, NULL);

    chars = TOMOE_DICT_GET_PRIVATE(dict)->letters;
    for (i = 0; i < chars->len; i++) {
        TomoeChar *chr = g_ptr_array_index (chars, i);
        if (0 == strcmp(tomoe_char_get_code(chr), code_point)) {
            return chr;
        }
    }

    return NULL;
}

const GPtrArray*
tomoe_dict_get_letters (TomoeDict *dict)
{
    g_return_val_if_fail(dict, NULL);
    return TOMOE_DICT_GET_PRIVATE(dict)->letters;
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
    g_ptr_array_foreach (priv->letters, tomoe_dict_collect_chars_by_reading,
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
typedef struct _ParseData
{
    TomoeDict        *dict;
    TomoeDictPrivate *priv;

    gboolean in_dict;
    gboolean in_literal;
    gboolean in_stroke;
    gboolean in_reading;
    gboolean in_meta;

    TomoeChar    *chr;
    TomoeWriting *writing;

    /* meta data */
    gchar *key;
    gchar *value;
} ParseData;

static void
start_element_handler (GMarkupParseContext *context,
                       const gchar         *element_name,
                       const gchar        **attr_names,
                       const gchar        **attr_values,
                       gpointer             user_data,
                       GError             **error)
{
#warning FIXME: need error check

    ParseData *data = user_data;

    if (!strcmp ("tomoe_dictionary", element_name)) {
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

    if (!strcmp ("character", element_name)) {
        data->chr = tomoe_char_new ();
        return;
    }

    if (!strcmp ("literal", element_name)) {
        data->in_literal = TRUE;
        return;
    }

    if (!strcmp ("strokelist", element_name)) {
        data->writing = tomoe_writing_new ();
        return;
    }

    if (!strcmp ("s", element_name)) {
        data->in_stroke = TRUE;
        return;
    }

    if (!strcmp ("readings", element_name)) {
        return;
    }

    if (!strcmp ("r", element_name)) {
        data->in_reading = TRUE;
        return;
    }

    if (!strcmp ("meta", element_name)) {
        data->in_meta = TRUE;
        return;
    }

    if (data->in_meta) {
        g_free (data->key);
        g_free (data->value);
        data->key   = g_strdup (element_name);
        data->value = NULL;
    }
}

static void
end_element_handler (GMarkupParseContext *context,
                     const gchar         *element_name,
                     gpointer             user_data,
                     GError             **error)
{
#warning FIXME: need error check

    ParseData *data = user_data;

    if (!strcmp ("tomoe_dictionary", element_name)) {
        data->in_dict = FALSE;
        return;
    }

    if (!strcmp ("character", element_name)) {
        if (tomoe_char_get_code (data->chr))
            g_ptr_array_add (data->priv->letters, data->chr);
        else
            g_object_unref (G_OBJECT (data->chr));
        data->chr = NULL;
        return;
    }

    if (!strcmp("literal", element_name)) {
        data->in_literal = FALSE;
        return;
    }

    if (!strcmp ("strokelist", element_name)) {
        if (data->chr && data->writing)
            tomoe_char_set_writing (data->chr, data->writing);
        data->writing = NULL;
        return;
    }

    if (!strcmp ("s", element_name)) {
        data->in_stroke = FALSE;
        return;
    }

    if (!strcmp ("readings", element_name)) {
        return;
    }

    if (!strcmp ("r", element_name)) {
        data->in_reading = FALSE;
        return;
    }

    if (!strcmp ("meta", element_name)) {
        data->in_meta = FALSE;
        return;
    }

    if (data->in_meta) {
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
#warning FIXME: need error check

    ParseData *data = user_data;

    if (data->in_literal && data->chr) {
        tomoe_char_set_code (data->chr, text);
        return;
    }

    /* FIXME! */
    if (data->in_stroke && data->chr) {
        const gchar *p = text;
        guint point_num = 0, k;

        /* count stroke points */
        for (; *p; p++)
            if (*p == '(') 
                point_num ++;

        /* parse stroke */
        p = text;
        for (k = 0; k < point_num; k++) {
            gint x = 0, y = 0;

            sscanf (p, " (%d %d)", &x, &y);
            if (k == 0)
                tomoe_writing_move_to (data->writing, x, y);
            else
                tomoe_writing_line_to (data->writing, x, y);

            p = strchr (p, ')') + 1;
        }
    }

    if (data->in_reading && data->chr) {
        TomoeReading *reading;
#warning FIXME: detect reading type?
        reading = tomoe_reading_new (TOMOE_READING_ON, text);
        tomoe_char_add_reading (data->chr, reading);
        g_object_unref (reading);
    }

    if (data->in_meta && data->chr) {
        g_free (data->value);
        data->value = g_strdup (text);
    }
}

static void
passthrough_handler (GMarkupParseContext *context,
                     const gchar         *text,
                     gsize                text_len,  
                     gpointer             user_data,
                     GError             **error)
{
#warning FIXME: need implement?
}

static void
error_handler (GMarkupParseContext *context,
               GError              *error,
               gpointer             user_data)
{
#warning FIXME: need error check
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

    f = fopen (priv->filename, "rb");
    g_return_val_if_fail (f, FALSE);

    data.dict    = dict;
    data.priv    = priv;
    data.in_dict = FALSE;
    data.in_literal = FALSE;
    data.in_stroke = FALSE;
    data.in_reading = FALSE;
    data.chr     = NULL;
    data.writing = NULL;
    data.key = NULL;
    data.value = NULL;

    context = g_markup_parse_context_new (&parser, 0, &data, NULL);

    while ((bytes = fread (buf, sizeof (char), 4096, f)) > 0) {
        GError *error = NULL;
        gboolean success;

        success = g_markup_parse_context_parse(context, buf, bytes, &error);
#warning FIXME
        if (!success) {
            g_warning("XML parse error!: %s", error->message);
            g_error_free(error);
            break;
        }
    }

    fclose (f);
    g_markup_parse_context_free (context);

    if (priv->letters)
        g_ptr_array_sort (priv->letters, letter_compare_func);

    return TRUE;
}



static gboolean
_write_readings (TomoeChar *chr, FILE *f)
{
    GList *node = (GList*) tomoe_char_get_readings (chr);
    const gchar *head = "    <readings>\n";
    const gchar *foot = "    </readings>\n";

    if (fwrite (head, strlen (head), 1, f) < 1) return FALSE;

    for (; node; node = g_list_next (node)) {
        TomoeReading *reading = node->data;
        gchar *str;

        if (!TOMOE_IS_READING (reading)) continue;

        str = g_markup_printf_escaped ("      <r>%s</r>\n",
                                       tomoe_reading_get_reading (reading));
        if (fwrite (str, strlen (str), 1, f) < 1) {
            g_free (str);
            return FALSE;
        }
        g_free (str);
    }

    if (fwrite (foot, strlen (foot), 1, f) < 1) return FALSE;

    return TRUE;
}

static gboolean
_write_writing (TomoeChar *chr, FILE *f)
{
    TomoeWriting *writing = tomoe_char_get_writing (chr);
    GList *stroke_list = (GList*) tomoe_writing_get_strokes (writing);
    gchar buf[256];

    if (!stroke_list) return TRUE;

    g_snprintf (buf, G_N_ELEMENTS (buf), "    <strokelist>\n");
    if (fwrite (buf, strlen (buf), 1, f) < 1) return FALSE;

    for (; stroke_list; stroke_list = g_list_next (stroke_list)) {
        GList *point_list = stroke_list->data;

        if (!point_list) continue;

        g_snprintf (buf, G_N_ELEMENTS (buf), "      <s>");
        if (fwrite (buf, strlen (buf), 1, f) < 1) return FALSE;

        for (; point_list; point_list = g_list_next (point_list)) {
            TomoePoint *p = point_list->data;

            if (!p) continue;

            g_snprintf (buf, G_N_ELEMENTS (buf), "(%d %d) ", p->x, p->y);
            if (fwrite (buf, strlen (buf), 1, f) < 1) return FALSE;
        }

        g_snprintf (buf, G_N_ELEMENTS (buf), "</s>\n");
        if (fwrite (buf, strlen (buf), 1, f) < 1) return FALSE;
    }

    g_snprintf (buf, G_N_ELEMENTS (buf), "    </strokelist>\n");
    if (fwrite (buf, strlen (buf), 1, f) < 1) return FALSE;

    return TRUE;
}

static void
_write_meta_datum (gpointer key, gpointer value, gpointer user_data)
{
    FILE *f = user_data;
    gchar *str;

    str = g_markup_printf_escaped ("      <%s>%s</%s>\n",
                                   (gchar*)key, (gchar*)value, (gchar*)key);
    fwrite (str, strlen (str), 1, f);
    g_free (str);
}

static gboolean
_write_meta_data (TomoeChar *chr, FILE *f)
{
    const gchar *tag1 = "    <meta>\n";
    const gchar *tag2 = "    </meta>\n";

    if (fwrite (tag1, strlen (tag1), 1, f) < 1) return FALSE;
    tomoe_char_meta_data_foreach (chr, _write_meta_datum, f);
    if (fwrite (tag2, strlen (tag2), 1, f) < 1) return FALSE;

    return TRUE;
}

static gboolean
_write_character (TomoeChar *chr, FILE *f)
{
    gchar *head;
    const gchar *foot = "  </character>\n";

    g_return_val_if_fail (TOMOE_IS_CHAR (chr), FALSE);

    /* open character element */
    head = g_markup_printf_escaped ("  <character>\n"
                                    "    <literal>%s</literal>\n",
                                    tomoe_char_get_code (chr));
    if (fwrite (head, strlen (head), 1, f) < 1) {
        g_free (head);
        return FALSE;
    } else {
        g_free (head);
    }

    /* reading */
    if (tomoe_char_get_readings (chr))
        if (!_write_readings (chr, f)) return FALSE;

    /* writing */
    if (tomoe_char_get_writing (chr))
        if (!_write_writing (chr, f)) return FALSE;

    /* meta */
#if 0
    if (tomoe_char_has_meta_data (chr))
#endif
        if (!_write_meta_data (chr, f)) return FALSE;

    /* close character element */
    if (fwrite (foot, strlen (foot), 1, f) < 1) return FALSE;

    return TRUE;
}

static void
tomoe_dict_save_xml (TomoeDict *dict)
{
    TomoeDictPrivate *priv;
    FILE *f;
    gchar *head;
    const gchar *foot = "</tomoe_dictionary>\n";
    guint i;

    g_return_if_fail (TOMOE_IS_DICT (dict));
    if (!tomoe_dict_is_editable (dict)) return;

    priv = TOMOE_DICT_GET_PRIVATE(dict);

    f = fopen (priv->filename, "wb");
    g_return_if_fail (f);

    /* write the header */
    head = g_markup_printf_escaped (
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone\"no\"?>\n"
        "<!DOCTYPE tomoe_dictionary SYSTEM \"tomoe-dict.dtd\">\n"
        "<tomoe_dictionary name=\"%s\">\n",
        priv->name);
    if (fwrite (head, strlen (head), 1, f) < 1) goto ERROR;

    /* write each characters */
    for (i = 0; i < priv->letters->len; i++) {
        TomoeChar* chr = (TomoeChar*)g_ptr_array_index (priv->letters, i);
        if (!_write_character (chr, f)) goto ERROR;
    }

    /* close root element */
    if (fwrite (foot, strlen (foot), 1, f) < 1) goto ERROR;

    /* clean */
    g_free (head);
    fclose (f);
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
