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

#define TOMOE_CHAR__USE_XML_METHODS
#define TOMOE_DICT__USE_XSL_METHODS
#include "tomoe-dict.h"
#include "tomoe-char.h"

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
    TomoeBool            editable;
    TomoeArray*          letters;
    xsltStylesheetPtr    metaXsl;
    char                *meta_xsl_file;
    TomoeBool            modified;
};

typedef struct _tomoe_metric tomoe_metric;

struct _tomoe_metric
{
    int    a;
    int    b;
    int    c;
    double d;
    int    e;
    double angle;
};

typedef struct _cand_priv cand_priv;

struct _cand_priv
{
    TomoeCandidate  *cand;
    int              index;
    IntArray        *adapted_strokes;
};

static cand_priv *cand_priv_new             (TomoeChar*       character,
                                             int              index);
static void       cand_priv_free            (cand_priv       *cand_p,
                                             TomoeBool        free_candidate);

static PointerArray
                 *get_candidates            (const TomoeDict *t_dict,
                                             TomoeStroke*     input_stroke,
                                             PointerArray    *cands);

static int        match_stroke_num          (const TomoeDict *t_dict,
                                             int              letter_index,
                                             int              input_stroke_num,
                                             IntArray        *adapted);

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
void              _parse_tomoe_dict         (TomoeDict*      t_dict,
                                             xmlNodePtr       root);
void              _parse_alien_dict         (TomoeDict*      t_dict,
                                             const char*      filename);


TomoeDict*
tomoe_dict_new (const char* filename, TomoeBool editable)
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

    if (0 == _check_dict_xsl (filename))
    {
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

    tomoe_array_sort (t_dict->letters);
    t_dict->filename = strdup (filename);

    for (i = 0; i < tomoe_array_size (t_dict->letters); i++)
    {
        TomoeChar *chr = (TomoeChar*)tomoe_array_get (t_dict->letters, i);
        tomoe_char_set_modified (chr, 0);
    }
    t_dict->modified = 0;

    return t_dict;
}

void
tomoe_dict_free (TomoeDict* t_dict)
{
    if (!t_dict) return;

    t_dict->ref --;
    if (t_dict->ref <= 0)
    {
        tomoe_array_free (t_dict->letters);
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
    int i, num;

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

    num = tomoe_array_size (t_dict->letters);
    for (i = 0; i < num; i++)
    {
        xmlNodePtr charNode = xmlNewChild (root, NULL, BAD_CAST "character", NULL);
        TomoeChar* chr = (TomoeChar*)tomoe_array_get (t_dict->letters, i);
        TomoeArray* readings = tomoe_char_get_readings (chr);
        TomoeGlyph* glyph = tomoe_char_get_glyph (chr);
        xmlNodePtr meta = tomoe_char_get_xml_meta (chr);
        const char* code = tomoe_char_get_code (chr);
        xmlNewChild (charNode, NULL, BAD_CAST "literal", BAD_CAST code);
        unsigned int k;

        if (glyph)
        {
            xmlNodePtr strokelistNode = xmlNewChild (charNode, NULL, BAD_CAST "strokelist", NULL);
            for (k = 0; k < glyph->stroke_num; k++)
            {
                unsigned int j;
                char buf[256]; /* FIXME overrun possible */
                strcpy (buf, "");
                for (j = 0; j < glyph->strokes[k].point_num; j++)
                {
                    char buf2[32];
                    sprintf (buf2, "(%d %d) ", glyph->strokes[k].points[j].x, glyph->strokes[k].points[j].y);
                    strcat (buf, buf2);
                }
                xmlNewChild (strokelistNode, NULL, BAD_CAST "s", BAD_CAST buf);
            }
        }
        if (readings)
        {
            unsigned int readings_num = tomoe_array_size (readings);
            xmlNodePtr readingsNode = xmlNewChild (charNode, NULL, BAD_CAST "readings", NULL);
            for (k = 0; k < readings_num; k++)
                xmlNewChild (readingsNode, NULL, BAD_CAST "r", tomoe_array_get (readings, k));
        }
        if (meta)
        {
            xmlAddChild (charNode, xmlCopyNode (meta, 1));
        }

        tomoe_char_set_modified (chr, 0);
    }

    xmlSaveFormatFileEnc(t_dict->filename, doc, "UTF-8", 1);
    xmlFreeDoc (doc);
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

TomoeBool
tomoe_dict_is_editable (TomoeDict* t_dict)
{
    if (!t_dict) return 0;
    return t_dict->editable;
}

TomoeBool
tomoe_dict_is_modified (TomoeDict *dict)
{
    if (!dict) return 0;
    return dict->modified;
}

void
tomoe_dict_set_modified (TomoeDict *dict, TomoeBool modified)
{
    if (!dict) return;
    dict->modified = modified;
}

int
tomoe_dict_get_size (TomoeDict* t_dict)
{
    if (!t_dict) return 0;
    return tomoe_array_size (t_dict->letters);
}

void
tomoe_dict_add_char (TomoeDict* t_dict, TomoeChar* add)
{
    if (!t_dict || !add) return;
    tomoe_char_set_dict_interface (add, &t_dict->dict_interface);
    tomoe_array_append (t_dict->letters, add);
    tomoe_array_sort (t_dict->letters);
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
    tomoe_array_remove (t_dict->letters, tomoe_dict_find_index (t_dict, remove));
    tomoe_dict_set_modified (t_dict, 1);
}

void
tomoe_dict_remove_by_index (TomoeDict* t_dict, int remove)
{
    if (!t_dict) return;
    tomoe_array_remove (t_dict->letters, remove);
    tomoe_dict_set_modified (t_dict, 1);
}

int
tomoe_dict_find_index (TomoeDict* t_dict, TomoeChar* find)
{
    if (!t_dict) return -1;
    return tomoe_array_find (t_dict->letters, find);
}

TomoeChar*
tomoe_dict_char_by_index (TomoeDict* t_dict, int index)
{
    if (!t_dict) return NULL;
    return tomoe_array_get (t_dict->letters, index);
}

TomoeArray*
tomoe_dict_get_letters (TomoeDict *t_dict)
{
    if (!t_dict) return NULL;
    return t_dict->letters;
}

TomoeArray*
tomoe_dict_search_by_strokes (const TomoeDict* t_dict, TomoeGlyph* input)
{
    TomoeArray* matched = tomoe_array_new((tomoe_compare_fn)tomoe_candidate_compare,
                                          (tomoe_addref_fn)tomoe_candidate_add_ref,
                                           (tomoe_free_fn)tomoe_candidate_free);
    unsigned int i, j; 
    IntArray *matches = NULL;
    PointerArray *cands = NULL;
    PointerArray *first_cands = NULL;
    unsigned int letters_num = 0;
    TomoeArray* letters = NULL;

    if (!input) return 0;
    if (!input->stroke_num) return 0;
    if (!t_dict) return 0;

    first_cands = _pointer_array_new ();
    letters = t_dict->letters;
    if (!letters) return 0;
    letters_num = tomoe_array_size (letters);

    for (i = 0; i < letters_num; i++)
    {
        TomoeChar* p = (TomoeChar*)tomoe_array_get (letters, i);
        cand_priv *cand = NULL;

        /* check for available glyph data */
        if (!tomoe_char_get_glyph (p))
            continue;

        /* check the number of stroke */
        if (input->stroke_num > tomoe_char_get_glyph (p)->stroke_num)
            continue;

        /* append a candidate to candidate list */
        cand = cand_priv_new ( p, i);
        _pointer_array_append_data (first_cands, cand);
    }

    cands = first_cands;
    first_cands = _pointer_array_ref (first_cands);

    for (i = 0; i < (unsigned int) input->stroke_num; i++)
    {
        PointerArray *verbose_cands;

        verbose_cands = get_candidates (t_dict, &input->strokes[i], cands);
        _pointer_array_unref (cands);
        cands = verbose_cands;
    }

    matches = _int_array_new ();
    for (i = 0; i < (unsigned int) cands->len; i++)
    {
        cand_priv *cand;
        IntArray *adapted;
        int pj;

        cand = cands->p[i];
        adapted = cand->adapted_strokes;
        pj = match_stroke_num (t_dict, cand->index, input->stroke_num, adapted);

        if (pj < 0)
        {
            continue;
        }

        if (pj != 0)
            cand->cand->score = cand->cand->score / pj;

        if (_int_array_find_data (matches, cand->index) < 0)
        {
            const TomoeChar* a = tomoe_array_get(letters, cand->index);
            TomoeBool b = TRUE;

            for (j = 0; j < (unsigned int) matches->len; j++)
            {
                const TomoeChar* b = tomoe_array_get(letters, matches->p[j]);
                if (!tomoe_char_compare (&a, &b))
                {
                    b = FALSE;
                    break;
                }
            }
            if (b)
            {
                matches = _int_array_append_data (matches, cand->index);
            }
        }
    }

    if (cands->len > 0)
    {
        for (i = 0; i < (unsigned int) cands->len; i++)
        {
            int index = ((cand_priv *)cands->p[i])->index;

            if (_int_array_find_data (matches, index) >= 0)
            {
                TomoeCandidate* cand = tomoe_candidate_new ();
                cand->character = tomoe_char_add_ref (((cand_priv*)cands->p[i])->cand->character);
                cand->score     = ((cand_priv *)cands->p[i])->cand->score;
                tomoe_array_append (matched, cand);
                tomoe_candidate_free (cand);
            }
        }
    }
    _int_array_unref (matches);

    tomoe_array_sort (matched);

    _pointer_array_unref (cands);

    for (i = 0; i < (unsigned int) first_cands->len; i++)
    {
        cand_priv_free (first_cands->p[i], TRUE);
    }

    _pointer_array_unref (first_cands);

    return matched;
}

TomoeArray*
tomoe_dict_search_by_reading (const TomoeDict* t_dict, const char* input)
{
    TomoeArray* reading = tomoe_array_new (NULL,
                                           (tomoe_addref_fn)tomoe_char_add_ref,
                                           (tomoe_free_fn)tomoe_char_free);
    int letter_num = tomoe_array_size (t_dict->letters);
    int i;

    for (i = 0; i < letter_num; i++)
    {
        TomoeChar* lttr = (TomoeChar*)tomoe_array_get (t_dict->letters, i);
        unsigned int j;
        unsigned int reading_num;
        TomoeArray* readings = tomoe_char_get_readings (lttr);

        /* check for available reading data */
        if (!readings)
            continue;

        reading_num = tomoe_array_size (readings);

        for (j = 0; j < reading_num; j++)
        {
            const char* r = (const char*)tomoe_array_get (readings, j);
            if (0 == strcmp (r, input))
                tomoe_array_append (reading, lttr);
        }
    }

    return reading;
}

xsltStylesheetPtr
tomoe_dict_get_meta_xsl (TomoeDict* t_dict)
{
    if (!t_dict) return NULL;
    return t_dict->metaXsl;
}

/*
 * *******************
 *  utility functions
 * *******************
 */

#define SQUARE_LENGTH(x, y) ((x) * (x) + (y) * (y))

static int
sq_dist (TomoePoint *p, TomoePoint *q)
{
    return SQUARE_LENGTH (p->x - q->x, p->y - q->y);
}


/*
 * *******************
 *  stroke functions.
 * *******************
 */
static int 
stroke_calculate_metrics (TomoeStroke *strk, tomoe_metric **met)
{
    unsigned int i = 0;
    TomoePoint p;
    TomoePoint q;
    tomoe_metric *m;

    if (!strk) return 0;

    m = calloc (strk->point_num - 1, sizeof (tomoe_metric));
 
    for (i = 0; i < strk->point_num - 1; i++)
    {
        p = strk->points[i];
        q = strk->points[i + 1];
        m[i].a     = q.x - p.x;
        m[i].b     = q.y - p.y;
        m[i].c     = q.x * p.y - q.y * p.x;
        m[i].d     = sqrt (m[i].a * m[i].a + m[i].b * m[i].b);
        m[i].e     = m[i].a * p.x + m[i].b * p.y;
        m[i].angle = atan2 (q.y - p.y, q.x - p.x);
    }
 
    *met = m;
    return strk->point_num - 1;
}

/*
 * **********************
 *  candidate private_functions.
 * **********************
 */

static cand_priv *
cand_priv_new (TomoeChar* character, int index)
{
    cand_priv *cand_p;

    cand_p                  = calloc (sizeof (cand_priv), 1);
    cand_p->cand            = calloc (sizeof (TomoeCandidate), 1);
    cand_p->cand->character = character;
    cand_p->cand->score     = 0;
    cand_p->index           = index;
    cand_p->adapted_strokes = _int_array_new ();

    return cand_p;
}

static void
cand_priv_free (cand_priv *cand_p, TomoeBool free_candidate)
{
    if (!cand_p) return;

    if (cand_p->adapted_strokes)
        _int_array_unref (cand_p->adapted_strokes);
    cand_p->adapted_strokes = NULL;

    if (free_candidate)
        free (cand_p->cand);
    cand_p->cand = NULL;

    free (cand_p);
}


/*
 * ************************************
 *  handwriting recognition functions.
 * ************************************
 */

static int
match_input_to_dict (TomoeStroke *input_stroke, TomoeStroke *dict_stroke)
{
    int i_nop = 0;              /* input stroke number of points */
    TomoePoint   *i_pts = NULL; /* input stroke points */
    tomoe_metric *i_met = NULL; /* input stroke metrics */
    int d_nop = 0;              /* dict stroke number of points */
    TomoePoint   *d_pts = NULL; /* dict stroke points */
    tomoe_metric *d_met = NULL; /* dict stroke metrics */
    int i_k_end = 0;
    int i_k = 0;
    int d_k = 0;
    int m = 0;
    TomoePoint i_pt;
    tomoe_metric i_me;
    TomoePoint d_pt;
    tomoe_metric d_me;
    int r = 0;
    int d = 0;
    int ret = 0;

    i_nop = input_stroke->point_num;
    i_pts = input_stroke->points;
    stroke_calculate_metrics (input_stroke, &i_met);
  
    d_nop = dict_stroke->point_num;
    d_pts = dict_stroke->points;
    stroke_calculate_metrics (dict_stroke, &d_met);

    /* 
     * if the length between last point and second last point is lesser than
     * LIMIT_LENGTH, the last itinerary assumes "hane".
     */
    if (sq_dist(&i_pts[i_nop - 1], &i_pts[i_nop - 2]) < LIMIT_LENGTH)
    {
        i_k_end = i_nop - 2;
    }
    else
    {
        i_k_end = i_nop - 1;
    }
  
    m = 0;
    for (i_k = 1; i_k < i_k_end; i_k++)
    {
        i_pt = i_pts[i_k];
        i_me = i_met[i_k];
        for (d_k = m; d_k < d_nop; d_k++)
        {
            d_pt = d_pts[d_k];
            d = sq_dist(&i_pt, &d_pt);
            if (d_k < d_nop - 1)
            {
                d_me = d_met[d_k];
                if (d < LIMIT_LENGTH &&
                    abs (i_me.angle - d_me.angle) < M_PI_2)
                {
                    m = d_k;
                    ret += d;
                    break;
                }
                else
                {
                    /* Distance between each characteristic points and line */
                    r = d_me.a * i_pt.x + d_me.b * i_pt.y - d_me.e;
                    d = abs (d_me.a * i_pt.y - d_me.b * i_pt.x - d_me.c);
                    if (0 <= r && r <= d_me.d * d_me.d &&
                        d < LIMIT_LENGTH * d_me.d &&
                        abs (i_me.angle - d_me.angle) < M_PI_2)
                    {
                        m = d_k;
                        ret += d;
                        break;
                    }
                }
            }
            else
            {
                if (d < LIMIT_LENGTH)
                {
                    m = d_k;
                    ret += d;
                    break;
                }
            }
        }
        if (d_k >= d_nop)
        {
            free (i_met);
            free (d_met);
            return -1;
        }
    }
    free (i_met);
    free (d_met);
    return ret;
}

static int
match_dict_to_input (TomoeStroke *dict_stroke, TomoeStroke *input_stroke)
{
    int           d_nop = 0;    /* dict stroke number of points */
    TomoePoint   *d_pts = NULL; /* dict stroke points */
    tomoe_metric *d_met = NULL; /* dict stroke metrics */
    int           i_nop = 0;    /* input stroke number of points */
    TomoePoint   *i_pts = NULL; /* input stroke points */
    tomoe_metric *i_met = NULL; /* input stroke metrics */
    int d_k_end = 0;
    int d_k = 0;
    int i_k = 0;
    int m = 0;
    TomoePoint d_pt;
    tomoe_metric d_me;
    TomoePoint i_pt;
    tomoe_metric i_me;
    int r = 0;
    int d = 0;
    int ret = 0;

    d_nop = dict_stroke->point_num;
    d_pts = dict_stroke->points;
    stroke_calculate_metrics (dict_stroke, &d_met);
    i_nop = input_stroke->point_num;
    i_pts = input_stroke->points;
    stroke_calculate_metrics (input_stroke, &i_met);

    /* 
     * if the length between last point and second last point is lesser than
     * LIMIT_LENGTH, the last itineraryassumes "hane".
     */
    if (sq_dist (&d_pts[d_nop - 1], &d_pts[d_nop - 2]) < LIMIT_LENGTH)
    {
        d_k_end = d_nop - 2;
    }
    else
    {
        d_k_end = d_nop - 1;
    }
    m = 0;
    for (d_k = 1; d_k < d_k_end - 1; d_k++) /* note difference: -1 */
    {
        d_pt = d_pts[d_k];
        d_me = d_met[d_k];
        for (i_k = m; i_k < i_nop; i_k++)
        {
            i_pt = i_pts[i_k];
            d = sq_dist (&d_pt, &i_pt);
            if (i_k < i_nop - 1)
            {
                i_me = i_met[i_k];
                if (d < LIMIT_LENGTH &&
                    abs (d_me.angle - i_me.angle) < M_PI_2)
                {
                    m = i_k;
                    ret += d;
                    break;
                }
                else
                {
                    /* Distance between each characteristic points and line */
                    r = i_me.a * d_pt.x + i_me.b * d_pt.y - i_me.e;
                    d = abs (i_me.a * d_pt.y - i_me.b * d_pt.x - i_me.c);
                    if (0 <= r && r <= i_me.d * i_me.d &&
                        d < LIMIT_LENGTH * i_me.d &&
                        abs (d_me.angle - i_me.angle) < M_PI_2)
                    {
                        m = i_k;
                        ret += d;
                        break;
                    }
                }
            }
            else
            {
                if (d < LIMIT_LENGTH)
                {
                    m = i_k;
                    ret += d;
                    break;
                }
            }
        }
        if (i_k >= i_nop)
        {
            free (i_met);
            free (d_met);
            return -1;
        }
    }
    free (i_met);
    free (d_met);
    return d;
}

static PointerArray *
get_candidates (const TomoeDict* t_dict, TomoeStroke *input_stroke, PointerArray *cands)
{
    PointerArray  *rtn_cands;
    cand_priv     *cand;
    int            cand_index = 0;
    TomoeChar*     lttr;
    unsigned int   strk_index = 0;
    TomoeStroke    dict_stroke;
    int            i_nop = 0;    /* input stroke number of points */
    TomoePoint    *i_pts = NULL; /* input stroke points */
    tomoe_metric  *i_met = NULL; /* input stroke metrics */
    int            d_nop = 0;    /* dict stroke number of points */
    TomoePoint    *d_pts = NULL; /* dict stroke points */
    tomoe_metric  *d_met = NULL; /* dict stroke metrics */

    rtn_cands = _pointer_array_new ();

    i_nop = input_stroke->point_num;
    i_pts = input_stroke->points;
    stroke_calculate_metrics (input_stroke, &i_met);

    for (cand_index = 0; cand_index < cands->len; cand_index++)
    {
        TomoeBool match_flag = FALSE;
        IntArray *adapted = NULL;
        TomoeArray* letters = t_dict->letters;

        cand = cands->p[cand_index];
        adapted = _int_array_copy (cand->adapted_strokes);
        lttr = tomoe_array_get (letters, cand->index);

        for (strk_index = 0;
             strk_index < tomoe_char_get_glyph (lttr)->stroke_num;
             strk_index++)
        {
            int d1 = 0, d2 = 0;
            int d3 = 0, d4 = 0;
            int score1 = 0, score2 = 0;
            int score3 = 0;

            if (_int_array_find_data (adapted, strk_index) >= 0)
            {
                continue;
            }

            dict_stroke = tomoe_char_get_glyph (lttr)->strokes[strk_index];
            d_nop = dict_stroke.point_num;
            d_pts = dict_stroke.points;
            stroke_calculate_metrics (&dict_stroke, &d_met);

            /*
             * Distance between the point and begining point.
             * Distance between the point and ending point.
             * Number of characteristic points.
             */
            d1 = sq_dist (&i_pts[0], &d_pts[0]);
            d2 = sq_dist (&i_pts[i_nop - 1], &d_pts[d_nop - 1]);
            score3 = (d1 + d2);
            cand->cand->score += score3;
            if (d1 > LIMIT_LENGTH ||
                d2 > LIMIT_LENGTH ||
                abs (d_nop - i_nop) > 3)
            {
                free (d_met);
                continue;
            }

            d3 = sq_dist (&i_pts[0], &i_pts[1]);
            d4 = sq_dist (&d_pts[0], &d_pts[1]);
            /* threshold is (angle of bigining line) % 45[degree] (PI/4)*/
            if (d1 > LIMIT_LENGTH &&
                d2 > LIMIT_LENGTH &&
                abs (d_met[0].angle - i_met[0].angle) > M_PI_4)
            {
                free (d_met);
                continue;
            }

            /*
             * Distance and angle of each characteristic points:
             * (Compare handwriting data with dictionary data)
             */
            score1 = match_input_to_dict (input_stroke, &dict_stroke);
            if (score1 < 0)
            {
                free (d_met);
                cand->cand->score = cand->cand->score * 2;
                continue;
            }
            cand->cand->score += score1;

            /*
             * Distance and angle of each characteristic points:
             * (Compare dictionary data with handwriting data)
             */
            score2 = match_dict_to_input (&dict_stroke, input_stroke);
            if (score2 < 0)
            {
                free (d_met);
                cand->cand->score = cand->cand->score * 2;
                continue;
            }
            cand->cand->score += score2;

            _int_array_append_data (cand->adapted_strokes, strk_index);
            match_flag = TRUE;

            strk_index = tomoe_char_get_glyph (lttr)->stroke_num;

            free (d_met);
        }

        if (match_flag)
        {
            _pointer_array_append_data (rtn_cands, cand);
        }
        _int_array_unref (adapted);
    }

    free (i_met);

    return rtn_cands;
}

static int
match_stroke_num (const TomoeDict* t_dict, int letter_index, int input_stroke_num, IntArray *adapted)
{
    TomoeArray* letters = t_dict->letters;
    int pj = 100;
    int adapted_num;
    TomoeChar* chr = (TomoeChar*)tomoe_array_get (letters, letter_index);
    int d_stroke_num = tomoe_char_get_glyph (chr)->stroke_num;

    if (!adapted)
        return -1;

    adapted_num = adapted->len;

    if (d_stroke_num - input_stroke_num >= 3)
    {
        int i, j;

        pj = 100;

        for (i = 0; i < adapted_num; i++)
        {
            j = adapted->p[i];
            if (j - pj >= 3)
            {
                return -1;
            }
            pj = j;
        }
    }
    return pj;
}

void
_parse_readings (xmlNodePtr node, TomoeChar* chr)
{
    xmlNodePtr child;
    for (child = node->children; child; child = child->next)
    {
        if (child->type == XML_ELEMENT_NODE)
        {
            TomoeArray* readings = tomoe_char_get_readings (chr);
            tomoe_array_append (readings, strdup ((const char*)child->children->content));
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
    for (child = node->children; child; child = child->next)
    {
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

    for (child = node->children; child; child = child->next)
    {
        if (child->type != XML_ELEMENT_NODE)
            continue;

        TomoeStroke* strk = &tomoe_char_get_glyph (lttr)->strokes[j];
        int point_num = 0;
        int k;
        const char* p = (const char*) child->children->content;

        /* count stroke points */
        for (; *p; p++)
            if (*p == '(') 
                point_num ++;

        /* parse stroke */
        p = (const char*) child->children->content;
        tomoe_stroke_init (strk, point_num);
        for (k = 0; k < point_num; k++)
        {
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
    t_dict->letters = tomoe_array_new((tomoe_compare_fn)tomoe_char_compare,
                                    (tomoe_addref_fn)tomoe_char_add_ref,
                                    (tomoe_free_fn)tomoe_char_free);

    /* parse xml tree */
    if (root && 0 == xmlStrcmp(root->name, BAD_CAST "tomoe_dictionary"))
    {
        xmlNodePtr node;
        xmlAttrPtr prop;

        /* read dictionary properties */
        for (prop = root->properties; prop; prop = prop->next)
        {
            if (0 == xmlStrcmp(prop->name, BAD_CAST "meta"))
            {
                const char* metaxsl = (const char*) prop->children->content;
                t_dict->meta_xsl_file = strdup (metaxsl);
                char* path = (char*)calloc (strlen (metaxsl) + strlen (TOMOEDATADIR) + 2, sizeof (char));
                strcpy (path, TOMOEDATADIR);
                strcat (path, "/");
                strcat (path, metaxsl);
                t_dict->metaXsl = xsltParseStylesheetFile (BAD_CAST path);
                free (path);
            }
            else if (0 == xmlStrcmp(prop->name, BAD_CAST "name"))
                t_dict->name = strdup ((const char*) prop->children->content);
        }

        /* read character nodes */
        for (node = root->children; node; node = node->next)
        {
            if (node->type != XML_ELEMENT_NODE)
                continue;

            if (0 == xmlStrcmp(node->name, BAD_CAST "character"))
            {
                TomoeChar* chr = tomoe_char_new (&t_dict->dict_interface);
                TomoeArray* readings = tomoe_array_new ((tomoe_compare_fn)tomoe_string_compare,
                                                        NULL,
                                                        (tomoe_free_fn)free);
                tomoe_char_set_readings (chr, readings);
                tomoe_array_free (readings);

                _parse_character (node, chr);
                tomoe_char_set_dict_interface (chr, &t_dict->dict_interface);
                if (tomoe_char_get_code (chr))
                    tomoe_array_append (t_dict->letters, chr);
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

/*
vi:ts=4:nowrap:ai:expandtab
*/
