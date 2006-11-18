/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <libxml/xmlreader.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <glib.h>

#define TOMOE_CHAR__USE_XML_METHODS
#define TOMOE_DICT__USE_XSL_METHODS
#include "tomoe-char.h"
#include "tomoe-dict.h"

struct _TomoeChar
{
    int                   ref;
    char                 *charCode;
    TomoeGlyph           *glyph;
    GPtrArray            *readings;
    xmlNodePtr            xmlMeta;
    char                 *meta;
    tomoe_dict_interface *parent;
    TomoeBool             modified;
};

TomoeStroke *
tomoe_stroke_new (void)
{
    TomoeStroke *strk = calloc (1, sizeof (TomoeStroke));

    if (strk)
        tomoe_stroke_init (strk, 0);

    return strk;
}

void
tomoe_stroke_init (TomoeStroke *strk, int point_num)
{
    if (!strk) return;

    strk->point_num = point_num;
    strk->points    = calloc (point_num, sizeof (TomoePoint));
}

void
tomoe_stroke_clear (TomoeStroke *strk)
{
    if (!strk) return;

    if (strk->points != NULL) {
        free (strk->points);
        strk->points = NULL;
    }
}

void
tomoe_stroke_free (TomoeStroke *strk)
{
    if (!strk) return;

    tomoe_stroke_clear (strk);
    free (strk);
}

TomoeGlyph *
tomoe_glyph_new (void)
{
    TomoeGlyph *glyph = calloc (1, sizeof (TomoeGlyph));

    if (glyph)
        tomoe_glyph_init (glyph, 0);

    return glyph;
}

void
tomoe_glyph_init (TomoeGlyph *glyph, int stroke_num)
{
    if (!glyph) return;

    glyph->stroke_num = stroke_num;
    glyph->strokes    = calloc (stroke_num, sizeof (TomoeStroke));
}

void
tomoe_glyph_clear (TomoeGlyph *glyph)
{
    unsigned int i;

    if (!glyph) return;

    if (glyph->strokes) {
        for (i = 0; i < glyph->stroke_num; i++)
            tomoe_stroke_clear (&glyph->strokes[i]);
        free (glyph->strokes);
        glyph->strokes = NULL;
    }

    glyph->stroke_num = 0;
}

void
tomoe_glyph_free (TomoeGlyph *glyph)
{
    if (!glyph) return;

    tomoe_glyph_clear (glyph);
    free (glyph);
}
#ifdef USE_TOMOE_OBJECT
TOMOE_CLASS_PRIV_BEGIN (TomoeCandidate)
    /* private class members
       int priv_number;
       ...
    */
TOMOE_CLASS_PRIV_END

/* TomoeCandidate_new, TomoeCandidate_getType (type registration) */
TOMOE_CLASS_IMPL (TomoeCandidate)

/* class methds */
/* ... */

/* TomoeCandidate *p = TomoeCandidate_new (); ...*/
#endif
TomoeChar*
tomoe_char_new (tomoe_dict_interface* dict)
{
    TomoeChar *p = calloc (1, sizeof (TomoeChar));

    if (!p) return NULL;

    p->ref       = 1;
    p->charCode  = NULL;
    p->glyph     = NULL;
    p->meta      = NULL;
    p->readings  = NULL;
    p->parent    = dict;
    p->modified  = 0;

    return p;
}

TomoeChar*
tomoe_char_add_ref (TomoeChar* t_char)
{
    if (!t_char) return NULL;
    t_char->ref ++;
    return t_char;
}

void
tomoe_char_free (TomoeChar *t_char)
{
    if (!t_char) return;

    t_char->ref --;
    if (t_char->ref <= 0) {
        if (t_char->charCode) free (t_char->charCode);
	if (t_char->glyph)    tomoe_glyph_free (t_char->glyph);
        if (t_char->xmlMeta)  xmlFreeNode (t_char->xmlMeta);
	if (t_char->meta)     free (t_char->meta);
	if (t_char->readings) {
	    g_ptr_array_foreach (t_char->readings, (GFunc) g_free, NULL);
	    g_ptr_array_free (t_char->readings, FALSE);
        }
	t_char->charCode = NULL;
	t_char->glyph    = NULL;
	t_char->xmlMeta  = NULL;
	t_char->meta     = NULL;
	t_char->readings = NULL;
        free (t_char);
    }
}

const char*
tomoe_char_get_code (const TomoeChar* t_char)
{
    if (!t_char) return NULL;
    return t_char->charCode;
}

void
tomoe_char_set_code (TomoeChar* t_char, const char* code)
{
    if (!t_char) return;
    free (t_char->charCode);
    t_char->charCode = code ? strdup (code) : NULL;
    tomoe_char_set_modified (t_char, 1);
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
    GPtrArray *readings;
    if (!t_char) return NULL;

    readings = g_ptr_array_new ();
    if (t_char->readings) {
        g_ptr_array_foreach (t_char->readings, _copy_reading_func, readings);
    }
    return readings;
}

void
tomoe_char_set_readings (TomoeChar* t_char, GPtrArray* readings)
{
    if (!t_char) return;

    if (t_char->readings) {
	g_ptr_array_foreach (t_char->readings, (GFunc) g_free, NULL);
        g_ptr_array_free (t_char->readings, TRUE);
        t_char->readings = NULL;
    }

    t_char->readings = g_ptr_array_new ();
    if (readings) {
        g_ptr_array_foreach (readings, _copy_reading_func, t_char->readings);
    }

    tomoe_char_set_modified(t_char, 1);
}

TomoeGlyph*
tomoe_char_get_glyph (TomoeChar* t_char)
{
    if (!t_char) return NULL;
    return t_char->glyph; 
}

void
tomoe_char_set_glyph (TomoeChar* t_char, TomoeGlyph* glyph)
{
    if (!t_char) return;
    tomoe_glyph_free (t_char->glyph);
    t_char->glyph = glyph; /* FIXME addRef */
    tomoe_char_set_modified(t_char, 1);
}

const char*
tomoe_char_get_meta (TomoeChar* t_char)
{
    xmlDocPtr doc;
    xmlDocPtr meta;
    const char* param[3];
    xmlNodePtr root;
    int len = 0;

    if (!t_char) return NULL;
    if (t_char->meta) return t_char->meta;
    if (!t_char->xmlMeta) return "";
    if (!t_char->parent->get_meta_xsl (t_char->parent->instance)) return "";

    /* create xml doc and include meta xml block */
    doc = xmlNewDoc(BAD_CAST "1.0");
    root = xmlNewNode(NULL, BAD_CAST "ch");
    param[0] = 0;

    xmlDocSetRootElement (doc, root);
    xmlAddChild (root, t_char->xmlMeta);

    /* translate xml meta to view text */
    meta = xsltApplyStylesheet (t_char->parent->get_meta_xsl (t_char->parent->instance), doc, param);

    /* save into character object */
    xmlChar* metaString = NULL;
    xsltSaveResultToString (&metaString, &len, meta, t_char->parent->get_meta_xsl (t_char->parent->instance));

    /* change of meta is invariant */
    t_char->meta = strdup ((const char*)metaString);

    xmlFreeDoc (meta);
    xmlUnlinkNode (t_char->xmlMeta);
    xmlFreeDoc (doc);

    xsltCleanupGlobals ();
    xmlCleanupParser ();

    return t_char->meta;
}

void
tomoe_char_set_dict_interface (TomoeChar *chr, tomoe_dict_interface *parent)
{
    if (!chr) return;
    chr->parent = parent;
}

TomoeBool
tomoe_char_is_editable (TomoeChar *chr)
{
    if (!chr) return 0;
    if (!chr->parent) return 1;
    return chr->parent->is_editable (chr->parent->instance);
}

TomoeBool
tomoe_char_is_modified (TomoeChar *chr)
{
    if (!chr) return 0;
    return chr->modified;
}

void
tomoe_char_set_modified (TomoeChar *chr, TomoeBool modified)
{
    if (!chr) return;
    chr->modified = modified;
    if (chr->parent && modified == 1)
        chr->parent->set_modified (chr->parent->instance, 1);
}

xmlNodePtr
tomoe_char_get_xml_meta (TomoeChar* t_char)
{
    if (!t_char) return NULL;
    return t_char->xmlMeta; /* TODO addRef?? */
}

void
tomoe_char_set_xml_meta (TomoeChar* t_char, xmlNodePtr meta)
{
    if (!t_char) return;
    if (t_char->xmlMeta) xmlFreeNode (t_char->xmlMeta);
    free (t_char->meta);
    t_char->xmlMeta = meta; /* TODO addRef?? */
    tomoe_char_set_modified(t_char, 1);
}

#if 0
void
tomoe_char_setMetaXsl (TomoeChar* t_char, xsltStylesheetPtr metaXsl)
{
    if (!t_char) return;
    t_char->metaXsl = metaXsl; /* TODO link to tomoe_dict instead of metaXsl */
}
#endif

gint
tomoe_char_compare (const TomoeChar *a, const TomoeChar *b)
{
    if (!a || !b) return 0;
    if (!a->charCode || !b->charCode) return 0;
    return strcmp (a->charCode, b->charCode);
}

TomoeCandidate*
tomoe_candidate_new (void)
{
    TomoeCandidate* cand;

    cand            = calloc (sizeof (TomoeCandidate), 1);
    cand->ref       = 1;
    cand->character = NULL;
    cand->score     = 0;

    return cand;
}

TomoeCandidate*
tomoe_candidate_add_ref (TomoeCandidate* t_cand)
{
    if (!t_cand) return NULL;
    t_cand->ref ++;
    return t_cand;
}

void
tomoe_candidate_free (TomoeCandidate* t_cand)
{
    if (!t_cand) return;
    t_cand->ref --;
    if (t_cand->ref <= 0) {
        tomoe_char_free (t_cand->character);
        free (t_cand);
    }
}

int
tomoe_candidate_compare (const TomoeCandidate *a, const TomoeCandidate *b)
{
    int score_a = a->score;
    int score_b = b->score;

    return score_a > score_b ? 1
        : score_a < score_b ? - 1
        : 0;
}

int
tomoe_string_compare (const char** a, const char** b)
{
    return strcmp(*a, *b);
}

/*
implementation_tomoe_array (TomoeCandidateArray,
                            const TomoeCandidate*,
                            tomoe_candidate_compare,
                            tomoe_candidate_addRef,
                            tomoe_candidate_free);
*/

/*
vi:ts=4:nowrap:ai:expandtab
*/
