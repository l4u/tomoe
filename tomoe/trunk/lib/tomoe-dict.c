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

#define TOMOE_CHAR__USE_XML_METHODS
#define TOMOE_DICT__USE_XSL_METHODS
#include "tomoe-dict.h"
#include "tomoe-char.h"
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
    xsltStylesheetPtr    metaXsl;
    char                *meta_xsl_filename;

    gboolean             editable;
    gboolean             modified;
};

enum
{
  PROP_0,
  PROP_NAME,
  PROP_FILENAME,
  PROP_LETTERS,
  PROP_META_XSL,
  PROP_META_XSL_FILENAME,
  PROP_EDITABLE,
  PROP_MODIFIED,
};

G_DEFINE_TYPE (TomoeDict, tomoe_dict, G_TYPE_OBJECT)

static void tomoe_dict_finalize       (GObject         *object);
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
static void letter_free_func          (gpointer         data,
                                       gpointer         user_data);
static gint letter_compare_func       (gconstpointer    a,
                                       gconstpointer    b);


static void
tomoe_dict_class_init (TomoeDictClass *klass)
{
    GObjectClass *gobject_class;
    GParamSpec *spec;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = tomoe_dict_finalize;
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
    priv->metaXsl = NULL;
    priv->meta_xsl_filename = NULL;
}


TomoeDict*
tomoe_dict_new (const char* filename, gboolean editable)
{
    TomoeDict* dict;
    TomoeDictPrivate *priv;

    int i;

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

    for (i = 0; i < priv->letters->len; i++) {
        TomoeChar *chr = (TomoeChar*)g_ptr_array_index (priv->letters, i);
        tomoe_char_set_modified (chr, 0);
    }
    priv->modified = FALSE;

    return dict;
}

static void
tomoe_dict_finalize (GObject *object)
{
    TomoeDict *dict;
    TomoeDictPrivate *priv;

    dict = TOMOE_DICT(object);
    priv = TOMOE_DICT_GET_PRIVATE(dict);

    g_free(priv->name);
    g_free(priv->filename);
    TOMOE_PTR_ARRAY_FREE_ALL(priv->letters, letter_free_func);
    if (priv->metaXsl)
        xsltFreeStylesheet(priv->metaXsl);
    g_free(priv->meta_xsl_filename);

    G_OBJECT_CLASS (tomoe_dict_parent_class)->finalize (object);
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
    if (priv->meta_xsl_filename)
        xmlNewProp (root, BAD_CAST "meta", BAD_CAST priv->meta_xsl_filename);

    num = priv->letters->len;
    for (i = 0; i < num; i++) {
        xmlNodePtr charNode = xmlNewChild (root, NULL, BAD_CAST "character", NULL);
        TomoeChar* chr = (TomoeChar*)g_ptr_array_index (priv->letters, i);
        GPtrArray *readings = tomoe_char_get_readings (chr);
        TomoeGlyph* glyph = tomoe_char_get_glyph (chr);
        xmlNodePtr meta = tomoe_char_get_xml_meta (chr);
        const char* code = tomoe_char_get_code (chr);
        xmlNewChild (charNode, NULL, BAD_CAST "literal", BAD_CAST code);
        unsigned int k;

        if (glyph) {
            xmlNodePtr strokelistNode = xmlNewChild (charNode, NULL, BAD_CAST "strokelist", NULL);
            for (k = 0; k < glyph->stroke_num; k++) {
                unsigned int j;
                char buf[256]; /* FIXME overrun possible */
                strcpy (buf, "");
                for (j = 0; j < glyph->strokes[k].point_num; j++) {
                    char buf2[32];
                    sprintf (buf2, "(%d %d) ", glyph->strokes[k].points[j].x, glyph->strokes[k].points[j].y);
                    strcat (buf, buf2);
                }
                xmlNewChild (strokelistNode, NULL, BAD_CAST "s", BAD_CAST buf);
            }
        }
        if (readings->len) {
            guint readings_num = readings->len;
            xmlNodePtr readingsNode = xmlNewChild (charNode, NULL, BAD_CAST "readings", NULL);
            for (k = 0; k < readings_num; k++)
                xmlNewChild (readingsNode, NULL, BAD_CAST "r", g_ptr_array_index (readings, k));
        }
        if (meta) {
            xmlAddChild (charNode, xmlCopyNode (meta, 1));
        }

        TOMOE_PTR_ARRAY_FREE_ALL (readings, g_free);
        tomoe_char_set_modified (chr, 0);
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
}

guint
tomoe_dict_get_size (TomoeDict* dict)
{
    g_return_val_if_fail(dict, 0);
    return TOMOE_DICT_GET_PRIVATE(dict)->letters->len;
}

void
tomoe_dict_add_char (TomoeDict* dict, TomoeChar* add)
{
    TomoeDictPrivate *priv;

    g_return_if_fail(dict);
    g_return_if_fail(add);

    tomoe_char_set_dict (add, dict);

    priv = TOMOE_DICT_GET_PRIVATE(dict);
    g_ptr_array_add (priv->letters, tomoe_char_add_ref (add));
    g_ptr_array_sort (priv->letters, letter_compare_func);
    tomoe_dict_set_modified (dict, TRUE);
}

void
tomoe_dict_insert (TomoeDict *dict, int position, TomoeChar *insert)
{
    if (!dict || !insert) return;
    /*tomoe_array_insert (dict->letters, position, insert);*/ 
    /* TODO do we need dict?? */ 
    tomoe_dict_set_modified (dict, TRUE);
}

void
tomoe_dict_remove_by_char (TomoeDict* dict, TomoeChar* remove)
{
    g_return_if_fail(dict);

    g_ptr_array_remove_index (TOMOE_DICT_GET_PRIVATE(dict)->letters,
                              tomoe_dict_find_index (dict, remove));
    tomoe_char_free (remove);
    tomoe_dict_set_modified (dict, TRUE);
}

void
tomoe_dict_remove_by_index (TomoeDict* dict, int remove)
{
    TomoeChar *c;
    TomoeDictPrivate *priv;

    g_return_if_fail(dict);

    priv = TOMOE_DICT_GET_PRIVATE(dict);
    c = (TomoeChar *) g_ptr_array_remove_index (priv->letters, remove);
    tomoe_char_free (c);
    tomoe_dict_set_modified (dict, TRUE);
}

glong
tomoe_dict_find_index (TomoeDict* dict, TomoeChar* find)
{
    TomoeDictPrivate *priv;
    guint i;

    g_return_val_if_fail(dict, -1);

    priv = TOMOE_DICT_GET_PRIVATE(dict);
    for (i = 0; i < priv->letters->len; i++) {
        TomoeChar *letter = g_ptr_array_index (priv->letters, i);
        if (tomoe_char_compare (letter, find))
            return (glong)i;
    }
    return -1;
}

TomoeChar*
tomoe_dict_char_by_index (TomoeDict* dict, int index)
{
    g_return_val_if_fail(dict, NULL);
    return g_ptr_array_index (TOMOE_DICT_GET_PRIVATE(dict)->letters, index);
}

const GPtrArray*
tomoe_dict_get_letters (TomoeDict *dict)
{
    g_return_val_if_fail(dict, NULL);
    return TOMOE_DICT_GET_PRIVATE(dict)->letters;
}

GPtrArray*
tomoe_dict_search_by_reading (const TomoeDict* dict, const char* input)
{
    TomoeDictPrivate *priv;
    GPtrArray *reading;
    guint letter_num;
    guint i;

    reading = g_ptr_array_new ();
    priv = TOMOE_DICT_GET_PRIVATE(dict);
    letter_num = priv->letters->len;

    for (i = 0; i < letter_num; i++) {
        TomoeChar *lttr = (TomoeChar*) g_ptr_array_index (priv->letters, i);
        guint reading_num, j;
        gboolean find = FALSE;
        GPtrArray *readings = tomoe_char_get_readings (lttr);

        /* check for available reading data */
        if (!readings->len) {
            TOMOE_PTR_ARRAY_FREE_ALL (readings, g_free);
            continue;
        }

        reading_num = readings->len;

        for (j = 0; j < reading_num; j++) {
            const char* r = (const char*) g_ptr_array_index (readings, j);
            if (0 == strcmp (r, input)) {
                find = TRUE;
                break;
            }
        }
        if (find)
            g_ptr_array_add (reading, tomoe_char_add_ref (lttr));
        TOMOE_PTR_ARRAY_FREE_ALL (readings, g_free);
    }

    return reading;
}

xsltStylesheetPtr
tomoe_dict_get_meta_xsl (TomoeDict* dict)
{
    g_return_val_if_fail(dict, NULL);
    return TOMOE_DICT_GET_PRIVATE(dict)->metaXsl;
}

static void
parse_readings (xmlNodePtr node, TomoeChar* chr)
{
    xmlNodePtr child;
    for (child = node->children; child; child = child->next) {
        if (child->type == XML_ELEMENT_NODE) {
            GPtrArray *readings = tomoe_char_get_readings (chr);
            g_ptr_array_add (readings, strdup ((const char*)child->children->content));
            tomoe_char_set_readings (chr, readings);
            TOMOE_PTR_ARRAY_FREE_ALL (readings, g_free);
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
            tomoe_char_set_xml_meta (lttr, xmlCopyNode (child, 1));
    }
}

static void
parse_strokelist (xmlNodePtr node, TomoeChar* lttr)
{
    TomoeGlyph *glyph;
    int stroke_num = 0;
    xmlNodePtr child;
    int j = 0;

    /* count strokes */
    for (child = node->children; child; child = child->next)
        if (child->type == XML_ELEMENT_NODE)
            stroke_num ++;

    if (stroke_num == 0)
        return;


    glyph = calloc(1, sizeof (TomoeGlyph));
    tomoe_glyph_init (glyph, stroke_num);
    tomoe_char_set_glyph (lttr, glyph);

    for (child = node->children; child; child = child->next) {
        TomoeStroke *strk = &tomoe_char_get_glyph (lttr)->strokes[j];
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
        tomoe_stroke_init (strk, point_num);
        for (k = 0; k < point_num; k++) {
            TomoePoint* pnt = &strk->points[k];

            sscanf (p, " (%d %d)", &pnt->x, &pnt->y);
            p = strchr (p, ')') + 1;
        }

        j++;
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
            if (0 == xmlStrcmp(prop->name, BAD_CAST "meta")) {
                const char* metaxsl = (const char*) prop->children->content;
                priv->meta_xsl_filename = strdup (metaxsl);
                char* path = (char*)calloc (strlen (metaxsl) + strlen (TOMOEDATADIR) + 2, sizeof (char));
                strcpy (path, TOMOEDATADIR);
                strcat (path, "/");
                strcat (path, metaxsl);
                priv->metaXsl = xsltParseStylesheetFile (BAD_CAST path);
                free (path);
            } else if (0 == xmlStrcmp(prop->name, BAD_CAST "name")) {
                priv->name = strdup ((const char*) prop->children->content);
            }
        }

        /* read character nodes */
        for (node = root->children; node; node = node->next) {
            if (node->type != XML_ELEMENT_NODE)
                continue;

            if (0 == xmlStrcmp(node->name, BAD_CAST "character")) {
                TomoeChar *chr = tomoe_char_new (dict);

                parse_character (node, chr);
                if (tomoe_char_get_code (chr))
                    g_ptr_array_add (priv->letters, tomoe_char_add_ref (chr));
                tomoe_char_free (chr);
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

static void
letter_free_func (gpointer data, gpointer user_data)
{
    TomoeChar *c = (TomoeChar *) data;

    tomoe_char_free (c);
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
