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

#include "tomoe-dict.h"
#include "tomoe-letter.h"
#include "array.h"

#define LIMIT_LENGTH ((300 * 0.25) * (300 * 0.25))
#define LINE_BUF_SIZE 4096
#define DICT_LETTER_INITIAL_SIZE 3049
#define DICT_LETTER_EXPAND_SIZE 10

struct _tomoe_dict
{
    char         *file_name;
    char         *dict_name;
    char         *encoding;
    char         *lang;
    tomoe_array* letters;
    unsigned int  ref_count;
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
    tomoe_candidate *cand;
    int              index;
    int_array       *adapted_strokes;
};

static cand_priv *cand_priv_new             (const char    *letter,
                                             int            index);
static void       cand_priv_free            (cand_priv     *cand_p,
                                             tomoe_bool     free_candidate);

static pointer_array
                 *get_candidates            (tomoe_dict*    dict,
                                             tomoe_stroke  *input_stroke,
                                             pointer_array *cands);

static int        match_stroke_num          (tomoe_dict*    dict,
                                             int            letter_index,
                                             int            input_stroke_num,
                                             int_array     *adapted);

tomoe_dict *
tomoe_dict_new (const char *filename)
{
    tomoe_dict *dict = calloc (1, sizeof (tomoe_dict));
    char *p = NULL;
    int stroke_num = 0;
    int point_num = 0;
    int j = 0, k = 0;
    tomoe_letter *lttr = NULL;
    tomoe_stroke *strk = NULL;
    tomoe_point  *pnt  = NULL;
    xmlTextReaderPtr reader;
    int res;
    int parse_mode = 0;

    LIBXML_TEST_VERSION

    if (!filename && !*filename) return NULL;
    if (!dict) return NULL;

    reader = xmlReaderForFile(filename, NULL, 0);
    if (!reader) return NULL;

    dict->letters = tomoe_array_new((tomoe_compare_fn)tomoe_letter_compare,
                                    (tomoe_addref_fn)tomoe_letter_addref,
                                    (tomoe_free_fn)tomoe_letter_free);

    // FIX-ME: clean up, error handling

    res = xmlTextReaderRead(reader);
    while (res == 1)
    {
        const xmlChar *name;

        name = xmlTextReaderConstName(reader);
        if (name == NULL)
            name = BAD_CAST "--";

        switch (parse_mode)
        {
        case 0:
            if (0 == xmlStrcmp(name, BAD_CAST "tomoe_dictionary"))
                parse_mode = 1;
            break;
        case 1:
            if (0 == xmlStrcmp(name, BAD_CAST "character"))
            {
                lttr = tomoe_letter_new();
                parse_mode = 2;
            }
            else if (0 == xmlStrcmp(name, BAD_CAST "tomoe_dictionary") && 
                     15 == xmlTextReaderNodeType(reader))
                parse_mode = 0;
            break;
        case 2:
            if (0 == xmlStrcmp(name, BAD_CAST "literal"))
                parse_mode = 3;
            else if (0 == xmlStrcmp(name, BAD_CAST "strokelist"))
            {
                sscanf(BAD_CAST xmlTextReaderGetAttribute(reader, BAD_CAST "count"), "%d", &stroke_num);
                lttr->c_glyph = calloc(1, sizeof (tomoe_glyph));
                tomoe_glyph_init(lttr->c_glyph, stroke_num);
                j = 0;
                parse_mode = 4;
            }
            else if (0 == xmlStrcmp(name, BAD_CAST "character") &&
                     15 == xmlTextReaderNodeType(reader))
            {
                tomoe_array_append(dict->letters, lttr);
                tomoe_letter_free(lttr);
                lttr = 0;
                parse_mode = 1;
            }
            break;
        case 3:
            if (0 == xmlStrcmp(name, BAD_CAST "literal") && 
                15 == xmlTextReaderNodeType(reader))
                parse_mode = 2;
            else
            {
                lttr->character = strdup (BAD_CAST xmlTextReaderConstValue(reader));
            }
            break;
        case 4:
            if (0 == xmlStrcmp(name, BAD_CAST "s"))
            {
                sscanf (xmlTextReaderGetAttribute(reader, BAD_CAST "count"), "%d", &point_num);
                parse_mode = 5;
            }
            else if (0 == xmlStrcmp(name, BAD_CAST "strokelist") && 
                     15 == xmlTextReaderNodeType(reader))
                parse_mode = 2;
            break;
        case 5:
            if (0 == xmlStrcmp(name, BAD_CAST "s") && 
                15 == xmlTextReaderNodeType(reader))
            {
                parse_mode = 4;
                j++;
            }
            else
            {
                strk = &lttr->c_glyph->strokes[j];
                tomoe_stroke_init (strk, point_num);
                p = BAD_CAST xmlTextReaderConstValue(reader);
                for (k = 0; k < point_num; k++)
                {
                    pnt = &strk->points[k];
                    sscanf (p, " (%d %d)", &pnt->x, &pnt->y);
                    p = strchr (p, ')') + 1;
                }
            }
            break;
        default:;
        }

        res = xmlTextReaderRead(reader);
    }

    xmlFreeTextReader(reader);

    dict->ref_count = 1;

    return dict;
}

void
tomoe_dict_free (tomoe_dict *dict)
{
    if (!dict) return;

    dict->ref_count--;
    if (dict->ref_count <= 0)
    {
        tomoe_array_free(dict->letters);
        free (dict);
    }
}

tomoe_dict *
tomoe_dict_addref (tomoe_dict *dict)
{
    if (!dict) return NULL;
    dict->ref_count++;
    return dict;
}


const char *
tomoe_dict_get_file_name (tomoe_dict *dict)
{
    if (!dict) return NULL;
    return dict->file_name;
}

const char *
tomoe_dict_get_name (tomoe_dict *dict)
{
    if (!dict) return NULL;
    return dict->dict_name;
}

tomoe_array *
tomoe_dict_get_letters (tomoe_dict *this)
{
    if (!this) return NULL;
    return this->letters;
}

tomoe_array *
tomoe_dict_get_matched (tomoe_dict* this, tomoe_glyph* input)
{
    tomoe_array* matched = tomoe_array_new((tomoe_compare_fn)tomoe_candidate_compare,
                                           NULL,
                                           (tomoe_free_fn)free);
    unsigned int i, j; 
    int_array *matches = NULL;
    pointer_array *cands = NULL;
    pointer_array *first_cands = NULL;
    unsigned int letters_num = 0;
    tomoe_array* letters = NULL;

    if (!input) return 0;
    if (!input->stroke_num) return 0;
    if (!this) return 0;

    first_cands = _pointer_array_new ();
    letters = tomoe_dict_get_letters (this);
    if (!letters) return 0;
    letters_num = tomoe_array_size (letters);

    for (i = 0; i < letters_num; i++)
    {
        tomoe_letter* p = (tomoe_letter*)tomoe_array_get (letters, i);

        cand_priv *cand = NULL;

        /* check the number of stroke */
        if (input->stroke_num > p->c_glyph->stroke_num)
        {
            continue;
        }

        /* append a candidate to candidate list */
        cand = cand_priv_new ((const char*) p->character, i);
        _pointer_array_append_data (first_cands, cand);
    }

    cands = first_cands;
    first_cands = _pointer_array_ref (first_cands);

    for (i = 0; i < (unsigned int) input->stroke_num; i++)
    {
        pointer_array *verbose_cands;

        verbose_cands = get_candidates (this, &input->strokes[i], cands);
        _pointer_array_unref (cands);
        cands = verbose_cands;
    }

    matches = _int_array_new ();
    for (i = 0; i < (unsigned int) cands->len; i++)
    {
        cand_priv *cand;
        int_array *adapted;
        int pj;

        cand = cands->p[i];
        adapted = cand->adapted_strokes;
        pj = match_stroke_num (this, cand->index, input->stroke_num, adapted);

        if (pj < 0)
        {
            continue;
        }

        if (pj != 0)
            cand->cand->score = cand->cand->score / pj;

        if (_int_array_find_data (matches, cand->index) < 0)
        {
            tomoe_letter* p0 = tomoe_array_get(letters, cand->index);
            tomoe_bool b = TRUE;

            for (j = 0; j < (unsigned int) matches->len; j++)
            {
                tomoe_letter* p1 = tomoe_array_get(letters, matches->p[j]);
                if (!strcmp (p0->character, p1->character))
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
                tomoe_candidate *cand;

                cand         = calloc (sizeof (tomoe_candidate), 1);
                cand->letter = ((cand_priv *)cands->p[i])->cand->letter;
                cand->score  = ((cand_priv *)cands->p[i])->cand->score;
                tomoe_array_append(matched, cand);

            }
        }
    }
    _int_array_unref (matches);

    tomoe_array_sort(matched);

    _pointer_array_unref (cands);

    for (i = 0; i < (unsigned int) first_cands->len; i++)
    {
        cand_priv_free (first_cands->p[i], TRUE);
    }

    _pointer_array_unref (first_cands);

    return matched;
}

/*
 * *******************
 *  utility functions
 * *******************
 */

#define SQUARE_LENGTH(x, y) ((x) * (x) + (y) * (y))

static int
sq_dist (tomoe_point *p, tomoe_point *q)
{
    return SQUARE_LENGTH (p->x - q->x, p->y - q->y);
}


/*
 * *******************
 *  stroke functions.
 * *******************
 */
static int 
stroke_calculate_metrics (tomoe_stroke *strk, tomoe_metric **met)
{
    unsigned int i = 0;
    tomoe_point p;
    tomoe_point q;
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
cand_priv_new (const char *letter, int index)
{
    cand_priv *cand_p;

    cand_p                  = calloc (sizeof (cand_priv), 1);
    cand_p->cand            = calloc (sizeof (tomoe_candidate), 1);
    cand_p->cand->letter    = letter;
    cand_p->cand->score     = 0;
    cand_p->index           = index;
    cand_p->adapted_strokes = _int_array_new ();

    return cand_p;
}

static void
cand_priv_free (cand_priv *cand_p, tomoe_bool free_candidate)
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
match_input_to_dict (tomoe_stroke *input_stroke, tomoe_stroke *dict_stroke)
{
    int i_nop = 0;              /* input stroke number of points */
    tomoe_point  *i_pts = NULL; /* input stroke points */
    tomoe_metric *i_met = NULL; /* input stroke metrics */
    int d_nop = 0;              /* dict stroke number of points */
    tomoe_point  *d_pts = NULL; /* dict stroke points */
    tomoe_metric *d_met = NULL; /* dict stroke metrics */
    int i_k_end = 0;
    int i_k = 0;
    int d_k = 0;
    int m = 0;
    tomoe_point i_pt;
    tomoe_metric i_me;
    tomoe_point d_pt;
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
match_dict_to_input (tomoe_stroke *dict_stroke, tomoe_stroke *input_stroke)
{
    int           d_nop = 0;    /* dict stroke number of points */
    tomoe_point  *d_pts = NULL; /* dict stroke points */
    tomoe_metric *d_met = NULL; /* dict stroke metrics */
    int           i_nop = 0;    /* input stroke number of points */
    tomoe_point  *i_pts = NULL; /* input stroke points */
    tomoe_metric *i_met = NULL; /* input stroke metrics */
    int d_k_end = 0;
    int d_k = 0;
    int i_k = 0;
    int m = 0;
    tomoe_point d_pt;
    tomoe_metric d_me;
    tomoe_point i_pt;
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

static pointer_array *
get_candidates (tomoe_dict* dict, tomoe_stroke *input_stroke, pointer_array *cands)
{
    pointer_array *rtn_cands;
    cand_priv     *cand;
    int            cand_index = 0;
    tomoe_letter*  lttr;
    unsigned int   strk_index = 0;
    tomoe_stroke   dict_stroke;
    int            i_nop = 0;    /* input stroke number of points */
    tomoe_point   *i_pts = NULL; /* input stroke points */
    tomoe_metric  *i_met = NULL; /* input stroke metrics */
    int            d_nop = 0;    /* dict stroke number of points */
    tomoe_point   *d_pts = NULL; /* dict stroke points */
    tomoe_metric  *d_met = NULL; /* dict stroke metrics */

    rtn_cands = _pointer_array_new ();

    i_nop = input_stroke->point_num;
    i_pts = input_stroke->points;
    stroke_calculate_metrics (input_stroke, &i_met);

    for (cand_index = 0; cand_index < cands->len; cand_index++)
    {
        tomoe_bool match_flag = FALSE;
        int_array *adapted = NULL;
        tomoe_array* letters = tomoe_dict_get_letters (dict);

        cand = cands->p[cand_index];
        adapted = _int_array_copy (cand->adapted_strokes);
        lttr = tomoe_array_get (letters, cand->index);

        for (strk_index = 0;
             strk_index < lttr->c_glyph->stroke_num;
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

            dict_stroke = lttr->c_glyph->strokes[strk_index];
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

            strk_index = lttr->c_glyph->stroke_num;

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
match_stroke_num (tomoe_dict* dict, int letter_index, int input_stroke_num, int_array *adapted)
{
    tomoe_array* letters = tomoe_dict_get_letters (dict);
    int pj = 100;
    int adapted_num;
    int d_stroke_num = ((tomoe_letter*)tomoe_array_get(letters, letter_index))->c_glyph->stroke_num;

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
