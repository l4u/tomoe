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

#define TOMOE_CHAR__USE_XML_METHODS
#define TOMOE_DICT__USE_XSL_METHODS
#include "tomoe-char.h"
#include "tomoe-dict.h"

struct _tomoe_char
{
    int                   ref;
    char                 *charCode;
    tomoe_glyph          *glyph;
    tomoe_array          *readings;
    xmlNodePtr            xmlMeta;
    char                 *meta;
    tomoe_dict_interface *parent;
    tomoe_bool            modified;
};

tomoe_stroke *
tomoe_stroke_new (void)
{
    tomoe_stroke *strk = calloc (1, sizeof (tomoe_stroke));

    if (strk)
        tomoe_stroke_init (strk, 0);

    return strk;
}

void
tomoe_stroke_init (tomoe_stroke *strk, int point_num)
{
    if (!strk) return;

    strk->point_num = point_num;
    strk->points    = calloc (point_num, sizeof (tomoe_point));
}

void
tomoe_stroke_clear (tomoe_stroke *strk)
{
    if (!strk) return;

    if (strk->points != NULL)
    {
        free (strk->points);
        strk->points = NULL;
    }
}

void
tomoe_stroke_free (tomoe_stroke *strk)
{
    if (!strk) return;

    tomoe_stroke_clear (strk);
    free (strk);
}

tomoe_glyph *
tomoe_glyph_new (void)
{
    tomoe_glyph *glyph = calloc (1, sizeof (tomoe_glyph));

    if (glyph)
        tomoe_glyph_init (glyph, 0);

    return glyph;
}

void
tomoe_glyph_init (tomoe_glyph *glyph, int stroke_num)
{
    if (!glyph) return;

    glyph->stroke_num = stroke_num;
    glyph->strokes    = calloc (stroke_num, sizeof (tomoe_stroke));
}

void
tomoe_glyph_clear (tomoe_glyph *glyph)
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
tomoe_glyph_free (tomoe_glyph *glyph)
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

/* class methods */
/* ... */

/* TomoeCandidate *p = TomoeCandidate_new (); ...*/
#endif
tomoe_char*
tomoe_char_new (tomoe_dict_interface* dict)
{
    tomoe_char *p = calloc (1, sizeof (tomoe_char));
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

tomoe_char*
tomoe_char_addRef (tomoe_char* this)
{
    if (!this) return NULL;
    this->ref ++;
    return this;
}

void
tomoe_char_free (tomoe_char *this)
{
    if (!this) return;

    this->ref --;
    if (this->ref <= 0)
    {
        free (this->charCode);
        tomoe_glyph_free (this->glyph);
        if (this->xmlMeta) xmlFreeNode (this->xmlMeta);
        free (this->meta);
        free (this);
    }
}

const char*
tomoe_char_getCode (const tomoe_char* this)
{
    if (!this) return NULL;
    return this->charCode;
}

void
tomoe_char_setCode (tomoe_char* this, const char* code)
{
    if (!this) return;
    free (this->charCode);
    this->charCode = code ? strdup (code) : NULL;
    tomoe_char_set_modified (this, 1);
}

tomoe_array*
tomoe_char_getReadings (tomoe_char* this)
{
    if (!this) return NULL;
    return this->readings;
}

void
tomoe_char_setReadings (tomoe_char* this, tomoe_array* readings)
{
    if (!this) return;
    tomoe_array_free (this->readings);
    this->readings = readings ? tomoe_array_addref (readings) : NULL;
    tomoe_char_set_modified(this, 1);
}

tomoe_glyph*
tomoe_char_getGlyph (tomoe_char* this)
{
    if (!this) return NULL;
    return this->glyph; 
}

void
tomoe_char_setGlyph (tomoe_char* this, tomoe_glyph* glyph)
{
    if (!this) return;
    tomoe_glyph_free (this->glyph);
    this->glyph = glyph; // FIXME addRef
    tomoe_char_set_modified(this, 1);
}

const char*
tomoe_char_getMeta (tomoe_char* this)
{
    xmlDocPtr doc;
    xmlDocPtr meta;
    const char* param[3];
    xmlNodePtr root;
    int len = 0;

    if (!this) return NULL;
    if (this->meta) return this->meta;
    if (!this->xmlMeta) return "";
    if (!this->parent->get_meta_xsl (this->parent->instance)) return "";

    // create xml doc and include meta xml block
    doc = xmlNewDoc(BAD_CAST "1.0");
    root = xmlNewNode(NULL, BAD_CAST "ch");
    param[0] = 0;

    xmlDocSetRootElement (doc, root);
    xmlAddChild (root, this->xmlMeta);

    // translate xml meta to view text
    meta = xsltApplyStylesheet (this->parent->get_meta_xsl (this->parent->instance), doc, param);

    // save into character object
    xmlChar* metaString = NULL;
    xsltSaveResultToString (&metaString, &len, meta, this->parent->get_meta_xsl (this->parent->instance));

    // change of meta is invariant
    this->meta = strdup ((const char*)metaString);

    xmlFreeDoc (meta);
    xmlUnlinkNode (this->xmlMeta);
    xmlFreeDoc (doc);

    xsltCleanupGlobals ();
    xmlCleanupParser ();

    return this->meta;
}

void
tomoe_char_set_dict_interface (tomoe_char *chr, tomoe_dict_interface *parent)
{
    if (!chr) return;
    chr->parent = parent;
}

tomoe_bool
tomoe_char_is_editable (tomoe_char *chr)
{
    if (!chr) return 0;
    if (!chr->parent) return 1;
    return chr->parent->get_editable (chr->parent->instance);
}

tomoe_bool
tomoe_char_get_modified (tomoe_char *chr)
{
    if (!chr) return 0;
    return chr->modified;
}

void
tomoe_char_set_modified (tomoe_char *chr, tomoe_bool modified)
{
    if (!chr) return;
    chr->modified = modified;
    if (chr->parent && modified == 1)
        chr->parent->set_modified (chr->parent->instance, 1);
}

xmlNodePtr
tomoe_char_getXmlMeta (tomoe_char* this)
{
    if (!this) return NULL;
    return this->xmlMeta; // TODO addRef??
}

void
tomoe_char_setXmlMeta (tomoe_char* this, xmlNodePtr meta)
{
    if (!this) return;
    if (this->xmlMeta) xmlFreeNode (this->xmlMeta);
    free (this->meta);
    this->xmlMeta = meta; // TODO addRef??
    tomoe_char_set_modified(this, 1);
}

/*void
tomoe_char_setMetaXsl (tomoe_char* this, xsltStylesheetPtr metaXsl)
{
    if (!this) return;
    this->metaXsl = metaXsl; // TODO link to tomoe_dict instead of metaXsl
}*/

int
tomoe_char_compare (const tomoe_char** a, const tomoe_char** b)
{
    return strcmp ((*a)->charCode, (*b)->charCode);
}

tomoe_candidate*
tomoe_candidate_new (void)
{
    tomoe_candidate* cand;
    cand            = calloc (sizeof (tomoe_candidate), 1);
    cand->ref       = 1;
    cand->character = NULL;
    cand->score     = 0;
    return cand;
}

tomoe_candidate*
tomoe_candidate_addRef (tomoe_candidate* this)
{
    if (!this) return NULL;
    this->ref ++;
    return this;
}

void
tomoe_candidate_free (tomoe_candidate* this)
{
    if (!this) return;
    this->ref --;
    if (this->ref <= 0)
    {
        tomoe_char_free (this->character);
        free (this);
    }
}

int
tomoe_candidate_compare (const tomoe_candidate** a, const tomoe_candidate** b)
{
    int score_a = a[0]->score;
    int score_b = b[0]->score;

    return score_a > score_b ? 1
        : score_a < score_b ? - 1
        : 0;
}

int
tomoe_string_compare (const char** a, const char** b)
{
    return strcmp(*a, *b);
}

//implementation_tomoe_array (tomoe_candidateArray, const tomoe_candidate*,
//    tomoe_candidate_compare, tomoe_candidate_addRef, tomoe_candidate_free);
