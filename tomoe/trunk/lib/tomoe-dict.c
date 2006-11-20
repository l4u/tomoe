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

struct _TomoeDict
{
    int                  ref;
    tomoe_dict_interface dict_interface;

    char*                filename;
    char*                name;
    char*                encoding;
    char*                lang;
    gboolean            editable;
    GPtrArray           *letters;
    xsltStylesheetPtr    metaXsl;
    char                *meta_xsl_file;
    gboolean            modified;
};

void              _parse_readings           (xmlNodePtr       node,
                                             TomoeChar*       chr);
void              _parse_meta               (xmlNodePtr       node,
                                             TomoeChar*       lttr);
void              _parse_character          (xmlNodePtr       node,
                                             TomoeChar*       lttr);
void              _parse_strokelist         (xmlNodePtr       node,
                                             TomoeChar*       lttr);
int               _check_dict_xsl           (const char*      filename);
/* tomoe_dict private methods */
void              _parse_tomoe_dict         (TomoeDict*       t_dict,
                                             xmlNodePtr       root);
void              _parse_alien_dict         (TomoeDict*       t_dict,
                                             const char*      filename);
static gint       _letter_compare_func      (gconstpointer    a,
                                             gconstpointer    b);

TomoeDict*
tomoe_dict_new (const char* filename, gboolean editable)
{
    TomoeDict* t_dict = NULL;
    int i;

    if (!filename && !*filename) return NULL;

    t_dict = calloc (1, sizeof (TomoeDict));
    if (!t_dict) return NULL;

    t_dict->ref                         = 1;
    t_dict->metaXsl                     = NULL;
    t_dict->meta_xsl_file               = NULL;
    t_dict->letters                     = NULL;
    t_dict->name                        = NULL;
    t_dict->filename                    = strdup (filename);
    t_dict->editable                    = editable;
    t_dict->dict_interface.instance     = t_dict;
    t_dict->dict_interface.get_meta_xsl = (tomoe_dict_interface_get_meta_xsl)tomoe_dict_get_meta_xsl;
    t_dict->dict_interface.is_editable  = (tomoe_dict_interface_is_editable)tomoe_dict_is_editable;
    t_dict->dict_interface.set_modified = (tomoe_dict_interface_set_modified)tomoe_dict_set_modified;

    if (0 == _check_dict_xsl (filename)) {
        /* native tomoe dictionary */
        xmlDocPtr doc = xmlParseFile (filename);

        if (doc)
            _parse_tomoe_dict (t_dict, xmlDocGetRootElement (doc));

        xmlFreeDoc (doc);
    }
    else /* non native dictionary */
        _parse_alien_dict (t_dict, filename);

    xsltCleanupGlobals ();
    xmlCleanupParser ();

    if (t_dict->letters) {
        g_ptr_array_sort (t_dict->letters, _letter_compare_func);
    }
    t_dict->filename = strdup (filename);

    for (i = 0; i < t_dict->letters->len; i++) {
        TomoeChar *chr = (TomoeChar*)g_ptr_array_index (t_dict->letters, i);
        tomoe_char_set_modified (chr, 0);
    }
    t_dict->modified = 0;

    return t_dict;
}

static void
_letter_free_func (gpointer data, gpointer user_data)
{
    TomoeChar *c = (TomoeChar *) data;

    tomoe_char_free (c);
}

void
tomoe_dict_free (TomoeDict* t_dict)
{
    if (!t_dict) return;

    t_dict->ref --;
    if (t_dict->ref <= 0) {
        TOMOE_PTR_ARRAY_FREE_ALL (t_dict->letters, _letter_free_func);
        free (t_dict->filename);
        if (t_dict->metaXsl)
            xsltFreeStylesheet (t_dict->metaXsl);
        free (t_dict);
    }
}

void
tomoe_dict_save (TomoeDict* t_dict)
{
    xmlDocPtr doc;
    const char* param[3];
    xmlNodePtr root;
    guint i, num;

    if (!t_dict) return;
    if (!tomoe_dict_is_editable (t_dict)) return;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root = xmlNewNode(NULL, BAD_CAST "tomoe_dictionary");
    param[0] = 0;

    xmlDocSetRootElement (doc, root);

    if (t_dict->name)
        xmlNewProp (root, BAD_CAST "name", BAD_CAST t_dict->name);
    if (t_dict->meta_xsl_file)
        xmlNewProp (root, BAD_CAST "meta", BAD_CAST t_dict->meta_xsl_file);

    num = t_dict->letters->len;
    for (i = 0; i < num; i++) {
        xmlNodePtr charNode = xmlNewChild (root, NULL, BAD_CAST "character", NULL);
        TomoeChar* chr = (TomoeChar*)g_ptr_array_index (t_dict->letters, i);
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

    xmlSaveFormatFileEnc(t_dict->filename, doc, "UTF-8", 1);
    xmlFreeDoc (doc);
    xmlCleanupCharEncodingHandlers();
    tomoe_dict_set_modified (t_dict, 0);
}

TomoeDict*
tomoe_dict_add_ref (TomoeDict* dict)
{
    if (!dict) return NULL;
    dict->ref ++;
    return dict;
}


const char*
tomoe_dict_get_filename (TomoeDict* t_dict)
{
    if (!t_dict) return NULL;
    return t_dict->filename;
}

const char*
tomoe_dict_get_name (TomoeDict* t_dict)
{
    if (!t_dict) return NULL;
    return t_dict->name;
}

gboolean
tomoe_dict_is_editable (TomoeDict* t_dict)
{
    if (!t_dict) return 0;
    return t_dict->editable;
}

gboolean
tomoe_dict_is_modified (TomoeDict *dict)
{
    if (!dict) return 0;
    return dict->modified;
}

void
tomoe_dict_set_modified (TomoeDict *dict, gboolean modified)
{
    if (!dict) return;
    dict->modified = modified;
}

guint
tomoe_dict_get_size (TomoeDict* t_dict)
{
    if (!t_dict) return 0;
    return t_dict->letters->len;
}

void
tomoe_dict_add_char (TomoeDict* t_dict, TomoeChar* add)
{
    if (!t_dict || !add) return;
    tomoe_char_set_dict_interface (add, &t_dict->dict_interface);
    g_ptr_array_add (t_dict->letters, tomoe_char_add_ref (add));
    g_ptr_array_sort (t_dict->letters, _letter_compare_func);
    tomoe_dict_set_modified (t_dict, 1);
}

void
tomoe_dict_insert (TomoeDict *dict, int position, TomoeChar *insert)
{
    if (!dict || !insert) return;
    /*tomoe_array_insert (t_dict->letters, position, insert);*/ 
    /* TODO do we need t_dict?? */ 
    tomoe_dict_set_modified (dict, 1);
}

void
tomoe_dict_remove_by_char (TomoeDict* t_dict, TomoeChar* remove)
{
    if (!t_dict) return;
    g_ptr_array_remove_index (t_dict->letters, tomoe_dict_find_index (t_dict, remove));
    tomoe_char_free (remove);
    tomoe_dict_set_modified (t_dict, 1);
}

void
tomoe_dict_remove_by_index (TomoeDict* t_dict, int remove)
{
    TomoeChar *c;

    if (!t_dict) return;
    c = (TomoeChar *) g_ptr_array_remove_index (t_dict->letters, remove);
    tomoe_char_free (c);
    tomoe_dict_set_modified (t_dict, 1);
}

glong
tomoe_dict_find_index (TomoeDict* t_dict, TomoeChar* find)
{
    guint i;

    if (!t_dict) return -1;
    for (i = 0; i < t_dict->letters->len; i++) {
        TomoeChar *letter = g_ptr_array_index (t_dict->letters, i);
        if (tomoe_char_compare (letter, find))
            return (glong)i;
    }
    return -1;
}

TomoeChar*
tomoe_dict_char_by_index (TomoeDict* t_dict, int index)
{
    if (!t_dict) return NULL;
    return g_ptr_array_index (t_dict->letters, index);
}

const GPtrArray*
tomoe_dict_get_letters (TomoeDict *t_dict)
{
    if (!t_dict) return NULL;
    return t_dict->letters;
}

GPtrArray*
tomoe_dict_search_by_reading (const TomoeDict* t_dict, const char* input)
{
    GPtrArray *reading = g_ptr_array_new ();
    guint letter_num = t_dict->letters->len;
    guint i;

    for (i = 0; i < letter_num; i++) {
        TomoeChar *lttr = (TomoeChar*) g_ptr_array_index (t_dict->letters, i);
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
tomoe_dict_get_meta_xsl (TomoeDict* t_dict)
{
    if (!t_dict) return NULL;
    return t_dict->metaXsl;
}

void
_parse_readings (xmlNodePtr node, TomoeChar* chr)
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

void
_parse_meta (xmlNodePtr node, TomoeChar* lttr)
{
#if 0
    xmlDocPtr doc;
    xmlDocPtr meta;
    const char* param[3];
    xmlNodePtr root;
    xmlNodePtr p = node->parent;
    int len = 0;

    if (!_metaXsl) return;

    /* create xml doc and include meta xml block */
    doc = xmlNewDoc(BAD_CAST "1.0");
    root = xmlNewNode(NULL, BAD_CAST "ch");
    param[0] = 0;

    xmlDocSetRootElement (doc, root);
    xmlUnlinkNode (node);
    xmlAddChild (root, node);

    xmlSubstituteEntitiesDefault (1);
    xmlLoadExtDtdDefaultValue = 1;

    /* translate xml meta to view text */
    meta = xsltApplyStylesheet (_metaXsl, doc, param);

    /* save into character object */
    xmlChar* metaString = NULL;
    xsltSaveResultToString (&metaString, &len, meta, _metaXsl);
    /*tomoe_char_setMeta (lttr, (const char*)metaString);*/
#endif
#if 0
    free (metaString);

    xmlFreeDoc (meta);

    xmlUnlinkNode (node);
    xmlAddChild (p, node);
    xmlFreeDoc (doc);
#endif
}

void
_parse_character (xmlNodePtr node, TomoeChar* lttr)
{
    xmlNodePtr child;
    for (child = node->children; child; child = child->next) {
        if (child->type != XML_ELEMENT_NODE)
            continue;

        if (0 == xmlStrcmp (child->name, BAD_CAST "readings"))
            _parse_readings (child, lttr);
        else if (0 == xmlStrcmp (child->name, BAD_CAST "strokelist"))
            _parse_strokelist (child, lttr);
        else if (0 == xmlStrcmp (child->name, BAD_CAST "literal"))
            tomoe_char_set_code (lttr, (const char*) child->children->content);
        else if (0 == xmlStrcmp (child->name, BAD_CAST "meta"))
            tomoe_char_set_xml_meta (lttr, xmlCopyNode (child, 1));
    }
}

void
_parse_strokelist (xmlNodePtr node, TomoeChar* lttr)
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

int
_check_dict_xsl (const char* filename)
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

void
_parse_tomoe_dict (TomoeDict* t_dict, xmlNodePtr root)
{
    t_dict->letters = g_ptr_array_new();

    /* parse xml tree */
    if (root && 0 == xmlStrcmp(root->name, BAD_CAST "tomoe_dictionary")) {
        xmlNodePtr node;
        xmlAttrPtr prop;

        /* read dictionary properties */
        for (prop = root->properties; prop; prop = prop->next) {
            if (0 == xmlStrcmp(prop->name, BAD_CAST "meta")) {
                const char* metaxsl = (const char*) prop->children->content;
                t_dict->meta_xsl_file = strdup (metaxsl);
                char* path = (char*)calloc (strlen (metaxsl) + strlen (TOMOEDATADIR) + 2, sizeof (char));
                strcpy (path, TOMOEDATADIR);
                strcat (path, "/");
                strcat (path, metaxsl);
                t_dict->metaXsl = xsltParseStylesheetFile (BAD_CAST path);
                free (path);
            } else if (0 == xmlStrcmp(prop->name, BAD_CAST "name")) {
                t_dict->name = strdup ((const char*) prop->children->content);
            }
        }

        /* read character nodes */
        for (node = root->children; node; node = node->next) {
            if (node->type != XML_ELEMENT_NODE)
                continue;

            if (0 == xmlStrcmp(node->name, BAD_CAST "character")) {
                TomoeChar *chr = tomoe_char_new (&t_dict->dict_interface);

                _parse_character (node, chr);
                if (tomoe_char_get_code (chr))
                    g_ptr_array_add (t_dict->letters, tomoe_char_add_ref (chr));
                tomoe_char_free (chr);
            }
        }
    }
}

void
_parse_alien_dict (TomoeDict* t_dict, const char* filename)
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
    _parse_tomoe_dict (t_dict, xmlDocGetRootElement (res));

    xsltFreeStylesheet (cur);
    xmlFreeDoc (res);
    xmlFreeDoc (doc);

}

static gint
_letter_compare_func (gconstpointer a, gconstpointer b)
{
    TomoeChar *ca = *(TomoeChar **) a;
    TomoeChar *cb = *(TomoeChar **) b;
    return tomoe_char_compare (ca, cb);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
