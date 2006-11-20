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
#include "glib-utils.h"

#define TOMOE_CHAR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_CHAR, TomoeCharPrivate))

typedef struct _TomoeCharPrivate	TomoeCharPrivate;
struct _TomoeCharPrivate
{
    char                 *charCode;
    TomoeGlyph           *glyph;
    GPtrArray            *readings;
    xmlNodePtr            xmlMeta;
    char                 *meta;
    TomoeDict            *parent;
    gboolean              modified;
};

G_DEFINE_TYPE (TomoeChar, tomoe_char, G_TYPE_OBJECT)

static void tomoe_char_dispose      (GObject *object);

static void
tomoe_char_class_init (TomoeCharClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose  = tomoe_char_dispose;

    g_type_class_add_private (gobject_class, sizeof (TomoeCharPrivate));
}

static void
tomoe_char_init (TomoeChar *t_char)
{
    TomoeCharPrivate *priv = TOMOE_CHAR_GET_PRIVATE (t_char);
    priv->charCode  = NULL;
    priv->glyph     = NULL;
    priv->meta      = NULL;
    priv->readings  = NULL;
    priv->parent    = NULL;
    priv->modified  = 0;
}

TomoeChar*
tomoe_char_new (TomoeDict *dict)
{
    TomoeChar *t_char;
    TomoeCharPrivate *priv;

    t_char = g_object_new(TOMOE_TYPE_CHAR, NULL);

    /* FIXME */
    priv = TOMOE_CHAR_GET_PRIVATE (t_char);
    priv->parent = dict;

    return t_char;
}

static void
tomoe_char_dispose (GObject *object)
{
    TomoeCharPrivate *priv = TOMOE_CHAR_GET_PRIVATE (object);

    if (priv->charCode)
        free (priv->charCode);
    if (priv->glyph)
        tomoe_glyph_free (priv->glyph);
    if (priv->xmlMeta)
        xmlFreeNode (priv->xmlMeta);
    if (priv->meta)
        free (priv->meta);
    if (priv->readings)
        TOMOE_PTR_ARRAY_FREE_ALL (priv->readings, g_free);

    priv->charCode = NULL;
    priv->glyph    = NULL;
    priv->xmlMeta  = NULL;
    priv->meta     = NULL;
    priv->readings = NULL;
    priv->parent   = NULL;

    G_OBJECT_CLASS (tomoe_char_parent_class)->dispose (object);
}

const char*
tomoe_char_get_code (const TomoeChar* t_char)
{
    TomoeCharPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CHAR (t_char), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);
    return priv->charCode;
}

void
tomoe_char_set_code (TomoeChar* t_char, const char* code)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (t_char));

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);
    free (priv->charCode);
    priv->charCode = code ? strdup (code) : NULL;
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
    TomoeCharPrivate *priv;
    GPtrArray *readings;

    g_return_val_if_fail (TOMOE_IS_CHAR (t_char), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    readings = g_ptr_array_new ();
    if (priv->readings) {
        g_ptr_array_foreach (priv->readings, _copy_reading_func, readings);
    }
    return readings;
}

void
tomoe_char_set_readings (TomoeChar* t_char, GPtrArray* readings)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (t_char));

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    if (priv->readings) {
        TOMOE_PTR_ARRAY_FREE_ALL (priv->readings, g_free);
        priv->readings = NULL;
    }

    priv->readings = g_ptr_array_new ();
    if (readings) {
        g_ptr_array_foreach (readings, _copy_reading_func, priv->readings);
    }

    tomoe_char_set_modified(t_char, 1);
}

TomoeGlyph*
tomoe_char_get_glyph (TomoeChar* t_char)
{
    TomoeCharPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CHAR (t_char), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    return priv->glyph; 
}

void
tomoe_char_set_glyph (TomoeChar* t_char, TomoeGlyph* glyph)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (t_char));

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    tomoe_glyph_free (priv->glyph);
    priv->glyph = glyph; /* FIXME addRef */
    tomoe_char_set_modified(t_char, 1);
}

const char*
tomoe_char_get_meta (TomoeChar* t_char)
{
    TomoeCharPrivate *priv;
    xmlDocPtr doc;
    xmlDocPtr meta;
    const char* param[3];
    xmlNodePtr root;
    int len = 0;

    g_return_val_if_fail (TOMOE_IS_CHAR (t_char), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    if (priv->meta) return priv->meta;
    if (!priv->xmlMeta) return "";
    if (!tomoe_dict_get_meta_xsl(priv->parent)) return "";

    /* create xml doc and include meta xml block */
    doc = xmlNewDoc(BAD_CAST "1.0");
    root = xmlNewNode(NULL, BAD_CAST "ch");
    param[0] = 0;

    xmlDocSetRootElement (doc, root);
    xmlAddChild (root, priv->xmlMeta);

    /* translate xml meta to view text */
    meta = xsltApplyStylesheet (tomoe_dict_get_meta_xsl(priv->parent), doc, param);

    /* save into character object */
    xmlChar* metaString = NULL;
    xsltSaveResultToString (&metaString, &len, meta, tomoe_dict_get_meta_xsl(priv->parent));

    /* change of meta is invariant */
    priv->meta = strdup ((const char*)metaString);

    xmlFreeDoc (meta);
    xmlUnlinkNode (priv->xmlMeta);
    xmlFreeDoc (doc);

    xsltCleanupGlobals ();
    xmlCleanupParser ();

    return priv->meta;
}

void
tomoe_char_set_dict (TomoeChar *t_char, TomoeDict *parent)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (t_char));

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    priv->parent = parent;
}

gboolean
tomoe_char_is_editable (TomoeChar *t_char)
{
    TomoeCharPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CHAR (t_char), FALSE);

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    if (!priv->parent) return TRUE;

    return tomoe_dict_is_editable(priv->parent);
}

gboolean
tomoe_char_is_modified (TomoeChar *t_char)
{
    TomoeCharPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CHAR (t_char), FALSE);

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    return priv->modified;
}

void
tomoe_char_set_modified (TomoeChar *t_char, gboolean modified)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (t_char));

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    priv->modified = modified;
    if (priv->parent && modified == 1)
        tomoe_dict_set_modified(priv->parent, TRUE);
}

xmlNodePtr
tomoe_char_get_xml_meta (TomoeChar* t_char)
{
    TomoeCharPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CHAR (t_char), NULL);

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    return priv->xmlMeta; /* TODO addRef?? */
}

void
tomoe_char_set_xml_meta (TomoeChar* t_char, xmlNodePtr meta)
{
    TomoeCharPrivate *priv;

    g_return_if_fail (TOMOE_IS_CHAR (t_char));

    priv = TOMOE_CHAR_GET_PRIVATE (t_char);

    if (priv->xmlMeta) xmlFreeNode (priv->xmlMeta);
    free (priv->meta);
    priv->xmlMeta = meta; /* TODO addRef?? */
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
    TomoeCharPrivate *priv_a, *priv_b;

    if (!a || !b) return 0;

    priv_a = TOMOE_CHAR_GET_PRIVATE (a);
    priv_b = TOMOE_CHAR_GET_PRIVATE (b);
    if (!priv_a || !priv_b) return 0;

    if (!priv_a->charCode || !priv_b->charCode) return 0;
    return strcmp (priv_a->charCode, priv_b->charCode);
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
        g_object_unref (G_OBJECT (t_cand->character));
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

/*
vi:ts=4:nowrap:ai:expandtab
*/
