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
#include <libxml/xmlreader.h>
#include <math.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
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

extern int xmlLoadExtDtdDefaultValue;

#define TOMOE_DICT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_DICT, TomoeDictPrivate))

typedef struct _TomoeDictPrivate TomoeDictPrivate;
struct _TomoeDictPrivate
{
    char*                filename;
    char*                name;
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

static void tomoe_dict_dispose        (GObject         *object);
static void tomoe_dict_set_property   (GObject         *object,
                                       guint            prop_id,
                                       const GValue    *value,
                                       GParamSpec      *pspec);
static void tomoe_dict_get_property   (GObject         *object,
                                       guint            prop_id,
                                       GValue          *value,
                                      GParamSpec      *pspec);
static void parse_readings            (xmlNodePtr       node,
                                       TomoeChar*       chr);
static void parse_character           (xmlNodePtr       node,
                                       TomoeChar*       lttr);
static void parse_strokelist          (xmlNodePtr       node,
                                       TomoeChar*       lttr);
static int  check_dict_xsl            (const char*      filename);
static void parse_tomoe_dict          (TomoeDict*       t_dict,
                                       xmlNodePtr       root);
static void parse_alien_dict          (TomoeDict*       t_dict,
                                       const char*      filename);
static gint letter_compare_func       (gconstpointer    a,
                                       gconstpointer    b);


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
    priv->name = NULL;
    priv->letters = NULL;
}


TomoeDict*
tomoe_dict_new (const char* filename, gboolean editable)
{
    TomoeDict* dict;
    TomoeDictPrivate *priv;

    if (!filename && !*filename) return NULL;

    dict = g_object_new(TOMOE_TYPE_DICT,
                        "filename", filename,
                        "editable", editable,
                        NULL);
    priv = TOMOE_DICT_GET_PRIVATE (dict);

    if (0 == check_dict_xsl (filename)) {
        /* native tomoe dictionary */
        xmlDocPtr doc = xmlParseFile (filename);

        if (doc)
            parse_tomoe_dict (dict, xmlDocGetRootElement (doc));

        xmlFreeDoc (doc);
    }
    else /* non native dictionary */
        parse_alien_dict (dict, filename);

    xsltCleanupGlobals ();
    xmlCleanupParser ();

    if (priv->letters) {
        g_ptr_array_sort (priv->letters, letter_compare_func);
    }

    priv->modified = FALSE;

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

static void
tomoe_dict_append_meta_data (gpointer key, gpointer value, gpointer user_data)
{
    const gchar *meta_key = key;
    const gchar *meta_value = value;
    xmlNodePtr parent = user_data;

    xmlNewChild (parent, NULL, BAD_CAST meta_key, BAD_CAST meta_value);
}

void
tomoe_dict_save (TomoeDict* dict)
{
    TomoeDictPrivate *priv;
    xmlDocPtr doc;
    const char* param[3];
    xmlNodePtr root;
    guint i, num;

    if (!dict) return;
    if (!tomoe_dict_is_editable (dict)) return;

    priv = TOMOE_DICT_GET_PRIVATE(dict);

    doc = xmlNewDoc(BAD_CAST "1.0");
    root = xmlNewNode(NULL, BAD_CAST "tomoe_dictionary");
    param[0] = 0;

    xmlDocSetRootElement (doc, root);

    if (priv->name)
        xmlNewProp (root, BAD_CAST "name", BAD_CAST priv->name);

    num = priv->letters->len;
    for (i = 0; i < num; i++) {
        xmlNodePtr charNode = xmlNewChild (root, NULL, BAD_CAST "character", NULL);
        TomoeChar* chr = (TomoeChar*)g_ptr_array_index (priv->letters, i);
        TomoeWriting* writing = tomoe_char_get_writing (chr);
        const char* code = tomoe_char_get_code (chr);
        xmlNewChild (charNode, NULL, BAD_CAST "literal", BAD_CAST code);
        unsigned int k;

        if (writing) {
            xmlNodePtr strokelistNode = xmlNewChild (charNode, NULL, BAD_CAST "strokelist", NULL);
            for (k = 0; k < tomoe_writing_get_number_of_strokes (writing); k++) {
                unsigned int j;
                char buf[256]; /* FIXME overrun possible */
                strcpy (buf, "");
                for (j = 0; j < tomoe_writing_get_number_of_points (writing, k); j++) {
                    gint x, y;
                    char buf2[32];
                    tomoe_writing_get_point (writing, k, j, &x, &y);
                    sprintf (buf2, "(%d %d) ", x, y);
                    strcat (buf, buf2);
                }
                xmlNewChild (strokelistNode, NULL, BAD_CAST "s", BAD_CAST buf);
            }
        }

        {
            const GList *readings, *reading;

            readings = tomoe_char_get_readings (chr);
            if (readings) {
                xmlNodePtr readingsNode;
                readingsNode = xmlNewChild (charNode, NULL,
                                            BAD_CAST "readings", NULL);
                for (reading = readings; reading; reading = reading->next) {
                    xmlNewChild (readingsNode, NULL, BAD_CAST "r",
                                 reading->data);
                }
            }
        }

        tomoe_char_meta_data_foreach (chr, tomoe_dict_append_meta_data,
                                      charNode);
    }

    xmlSaveFormatFileEnc(priv->filename, doc, "UTF-8", 1);
    xmlFreeDoc (doc);
    xmlCleanupCharEncodingHandlers();
    tomoe_dict_set_modified (dict, 0);
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

static void
parse_readings (xmlNodePtr node, TomoeChar* chr)
{
    xmlNodePtr child;
    for (child = node->children; child; child = child->next) {
        if (child->type == XML_ELEMENT_NODE) {
            TomoeReading *reading;
            reading =
                tomoe_reading_new (TOMOE_READING_ON,
                                   (const gchar *)child->children->content);
            tomoe_char_add_reading (chr, reading);
            g_object_unref (reading);
        }
    }
}

static void
parse_meta (xmlNodePtr node, TomoeChar *chr)
{
    xmlNodePtr child;
    for (child = node->children; child; child = child->next) {
        if (child->type == XML_ELEMENT_NODE) {
            const gchar *key, *value;
            key = (const gchar *)child->name;

            if (child->children) {
                value = (const gchar *)child->children->content;
                if (value)
                    tomoe_char_register_meta_data (chr, key, value);
            }
        }
    }
}

static void
parse_character (xmlNodePtr node, TomoeChar* lttr)
{
    xmlNodePtr child;
    for (child = node->children; child; child = child->next) {
        if (child->type != XML_ELEMENT_NODE)
            continue;

        if (0 == xmlStrcmp (child->name, BAD_CAST "readings"))
            parse_readings (child, lttr);
        else if (0 == xmlStrcmp (child->name, BAD_CAST "strokelist"))
            parse_strokelist (child, lttr);
        else if (0 == xmlStrcmp (child->name, BAD_CAST "literal"))
            tomoe_char_set_code (lttr, (const char*) child->children->content);
        else if (0 == xmlStrcmp (child->name, BAD_CAST "meta"))
            parse_meta (child, lttr);
    }
}

static void
parse_strokelist (xmlNodePtr node, TomoeChar* lttr)
{
    TomoeWriting *writing;
    int stroke_num = 0;
    xmlNodePtr child;

    /* count strokes */
    for (child = node->children; child; child = child->next)
        if (child->type == XML_ELEMENT_NODE)
            stroke_num ++;

    if (stroke_num == 0)
        return;

    writing = tomoe_writing_new ();
    tomoe_char_set_writing (lttr, writing);

    for (child = node->children; child; child = child->next) {
        int point_num = 0;
        int k;
        const char *p;

        if (child->type != XML_ELEMENT_NODE)
            continue;

        p = (const char*) child->children->content;
        /* count stroke points */
        for (; *p; p++)
            if (*p == '(') 
                point_num ++;

        /* parse stroke */
        p = (const char*) child->children->content;
        for (k = 0; k < point_num; k++) {
            gint x = 0, y = 0;

            sscanf (p, " (%d %d)", &x, &y);
            if (k == 0)
                tomoe_writing_move_to (writing, x, y);
            else
                tomoe_writing_line_to (writing, x, y);

            p = strchr (p, ')') + 1;
        }
    }
}

static int
check_dict_xsl (const char* filename)
{
    char* xslname = NULL;
    int len;
    FILE* f;

    if (!(xslname = strdup(filename))) return 0;
    if ((len = strlen(xslname)) < 3) return 0;

    xslname[len - 2] = 's'; /* xml > xsl */

    f = fopen(xslname, "r");
    free(xslname);
    if (!f)
        return 0; /* no xsl available */
    fclose(f);

    return 1;
}

static void
parse_tomoe_dict (TomoeDict* dict, xmlNodePtr root)
{
    TomoeDictPrivate *priv;

    priv = TOMOE_DICT_GET_PRIVATE(dict);
    priv->letters = g_ptr_array_new();

    /* parse xml tree */
    if (root && 0 == xmlStrcmp(root->name, BAD_CAST "tomoe_dictionary")) {
        xmlNodePtr node;
        xmlAttrPtr prop;

        /* read dictionary properties */
        for (prop = root->properties; prop; prop = prop->next) {
            if (0 == xmlStrcmp(prop->name, BAD_CAST "name")) {
                priv->name = g_strdup ((const char*) prop->children->content);
            }
        }

        /* read character nodes */
        for (node = root->children; node; node = node->next) {
            if (node->type != XML_ELEMENT_NODE)
                continue;

            if (0 == xmlStrcmp(node->name, BAD_CAST "character")) {
                TomoeChar *chr = tomoe_char_new ();

                parse_character (node, chr);
                if (tomoe_char_get_code (chr))
                    g_ptr_array_add (priv->letters, g_object_ref (G_OBJECT (chr)));
                g_object_unref (G_OBJECT (chr));
            }
        }
    }
}

static void
parse_alien_dict (TomoeDict* dict, const char* filename)
{
    char* xslname = strdup (filename);
    int len;
    xsltStylesheetPtr cur = NULL;
    xmlDocPtr doc, res;
    const char* param[1];
    param[0] = NULL;

    if (!xslname) return;
    if ((len = strlen (xslname)) < 3) return;
    xslname[len - 2] = 's'; /* xml > xsl */

    xmlSubstituteEntitiesDefault (1);
    xmlLoadExtDtdDefaultValue = 1;
    cur = xsltParseStylesheetFile (BAD_CAST xslname);
    doc = xmlParseFile (filename);
    /* translate to native */
    res = xsltApplyStylesheet (cur, doc, param);

    /* load native dictionary */
    parse_tomoe_dict (dict, xmlDocGetRootElement (res));

    xsltFreeStylesheet (cur);
    xmlFreeDoc (res);
    xmlFreeDoc (doc);

}

static gint
letter_compare_func (gconstpointer a, gconstpointer b)
{
    TomoeChar *ca = *(TomoeChar **) a;
    TomoeChar *cb = *(TomoeChar **) b;
    return tomoe_char_compare (ca, cb);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
