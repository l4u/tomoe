/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Kouhei Sutou <kou@cozmixng.org>
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
#include <math.h>
#include "tomoe-char.h"
#include "tomoe-dict.h"
#include "tomoe-candidate.h"
#include "tomoe-recognizer-simple-logic.h"

#define LIMIT_LENGTH ((300 * 0.25) * (300 * 0.25))

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
    gint             index;
    GArray          *adapted_strokes;
};

static cand_priv *cand_priv_new               (TomoeChar   *character,
                                               int          index);
static void       cand_priv_free              (cand_priv   *cand_p);
static GPtrArray *get_candidates              (TomoeStroke *input_stroke,
                                               GPtrArray   *cands);
static gint       match_stroke_num            (TomoeDict   *dict,
                                               int          letter_index,
                                               int          input_stroke_num,
                                               GArray      *adapted);
static gboolean  _g_array_has_this_int_value  (GArray      *a,
                                               gint         i);
static GArray   *_g_array_copy_int_value      (GArray      *a);

static gint
_candidate_compare_func (gconstpointer a, gconstpointer b)
{
    return tomoe_candidate_compare (a, b);
}

GPtrArray *
_tomoe_recognizer_simple_get_candidates (void *context, TomoeDict *dict, TomoeGlyph *input)
{
    /* TomoeRecognizerSimple *recognizer = context; */
    GPtrArray *matched = g_ptr_array_new ();
    guint i, j;
    GArray *matches = NULL;
    GPtrArray *cands = NULL;
    GPtrArray *first_cands = NULL;
    guint letters_num = 0;
    const GPtrArray *letters = NULL;

    if (!input) return 0;
    if (!input->stroke_num) return 0;
    if (!dict) return 0;

    first_cands = g_ptr_array_new ();
    letters = tomoe_dict_get_letters(dict);
    if (!letters) return NULL;
    letters_num = letters->len;

    for (i = 0; i < letters_num; i++) {
        TomoeChar *p = (TomoeChar *) g_ptr_array_index (letters, i);
        cand_priv *cand = NULL;

        /* check for available glyph data */
        if (!tomoe_char_get_glyph(p))
            continue;

        /* check the number of stroke */
        if (input->stroke_num > tomoe_char_get_glyph(p)->stroke_num)
            continue;

        /* append a candidate to candidate list */
        cand = cand_priv_new (p, i);
        g_ptr_array_add (first_cands, cand);
    }

    /* Ugly hack! */
    cands = first_cands;
    cands = get_candidates (&input->strokes[0], first_cands);
    for (i = 1; i < (guint)input->stroke_num; i++) {
        GPtrArray *tmp;
        tmp = get_candidates(&input->strokes[i], cands);
        g_ptr_array_free (cands, TRUE);
        cands = tmp;
    }

    matches = g_array_new (FALSE, FALSE, sizeof (gint));
    for (i = 0; i < (guint)cands->len; i++) {
        cand_priv *cand;
        GArray *adapted;
        int pj;

        cand = g_ptr_array_index (cands, i);
        adapted = cand->adapted_strokes;
        pj = match_stroke_num (dict, cand->index, input->stroke_num, adapted);

        if (pj < 0)
            continue;

        if (pj != 0)
            tomoe_candidate_set_score (
                cand->cand,
                tomoe_candidate_get_score (cand->cand) / pj);

        if (!_g_array_has_this_int_value (matches, cand->index)) {
            const TomoeChar *a = g_ptr_array_index (letters, cand->index);
            gboolean f = TRUE;

            for (j = 0; j < (guint)matches->len; j++) {
                const TomoeChar *b = g_ptr_array_index (letters, g_array_index (matches, gint, j));
                if (!tomoe_char_compare(a, b)) {
                    f = FALSE;
                    break;
                }
            }

            if (f) {
                g_array_append_val (matches, cand->index);
            }
        }
    }

    if (cands->len > 0) {
        for (i = 0; i < (guint)cands->len; i++) {
            cand_priv *cand_p = g_ptr_array_index (cands, i);
            gint index = cand_p->index;

            if (_g_array_has_this_int_value (matches, index)) {
                TomoeCandidate *c = cand_p->cand;
                TomoeCandidate *cand = tomoe_candidate_new (tomoe_candidate_get_char (c));
                tomoe_candidate_set_score (cand, tomoe_candidate_get_score (c));
                g_ptr_array_add (matched, cand);
            }
        }
    }
    g_array_free (matches, TRUE);

    g_ptr_array_sort (matched, _candidate_compare_func);

    g_ptr_array_foreach (first_cands, (GFunc) cand_priv_free, NULL);
    g_ptr_array_free (first_cands, TRUE);

    return matched;
}

/*
 * *******************
 *  stroke functions.
 * *******************
 */
static gint
stroke_calculate_metrics (TomoeStroke *strk, tomoe_metric **met)
{
    guint i = 0;
    TomoePoint p;
    TomoePoint q;
    tomoe_metric *m;

    if (!strk) return 0;

    m = g_new (tomoe_metric, strk->point_num - 1);
 
    for (i = 0; i < strk->point_num - 1; i++) {
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

    cand_p                  = g_new (cand_priv, 1);
    cand_p->cand            = tomoe_candidate_new (character);
    cand_p->index           = index;
    cand_p->adapted_strokes = g_array_new (FALSE, FALSE, sizeof (gint));

    return cand_p;
}

static void
cand_priv_free (cand_priv *cand_p)
{
    if (!cand_p) return;

    g_array_free (cand_p->adapted_strokes, TRUE);
    cand_p->adapted_strokes = NULL;

    g_object_unref (G_OBJECT (cand_p->cand));
    cand_p->cand = NULL;

    g_free (cand_p);
}


/*
 * *******************
 *  utility functions
 * *******************
 */

#define SQUARE_LENGTH(x, y) ((x) * (x) + (y) * (y))

static gint
sq_dist (TomoePoint *p, TomoePoint *q)
{
    return SQUARE_LENGTH (p->x - q->x, p->y - q->y);
}

/*
 * ************************************
 *  handwriting recognition functions.
 * ************************************
 */

static gint
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
    if (sq_dist(&i_pts[i_nop - 1], &i_pts[i_nop - 2]) < LIMIT_LENGTH) {
        i_k_end = i_nop - 2;
    } else {
        i_k_end = i_nop - 1;
    }
  
    m = 0;
    for (i_k = 1; i_k < i_k_end; i_k++) {
        i_pt = i_pts[i_k];
        i_me = i_met[i_k];
        for (d_k = m; d_k < d_nop; d_k++) {
            d_pt = d_pts[d_k];
            d = sq_dist(&i_pt, &d_pt);
            if (d_k < d_nop - 1) {
                d_me = d_met[d_k];
                if (d < LIMIT_LENGTH &&
                    abs (i_me.angle - d_me.angle) < M_PI_2) {
                    m = d_k;
                    ret += d;
                    break;
                } else {
                    /* Distance between each characteristic points and line */
                    r = d_me.a * i_pt.x + d_me.b * i_pt.y - d_me.e;
                    d = abs (d_me.a * i_pt.y - d_me.b * i_pt.x - d_me.c);
                    if (0 <= r && r <= d_me.d * d_me.d &&
                        d < LIMIT_LENGTH * d_me.d &&
                        abs (i_me.angle - d_me.angle) < M_PI_2) {
                        m = d_k;
                        ret += d;
                        break;
                    }
                }
            } else {
                if (d < LIMIT_LENGTH) {
                    m = d_k;
                    ret += d;
                    break;
                }
            }
        }
        if (d_k >= d_nop) {
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
    if (sq_dist (&d_pts[d_nop - 1], &d_pts[d_nop - 2]) < LIMIT_LENGTH) {
        d_k_end = d_nop - 2;
    } else {
        d_k_end = d_nop - 1;
    }
    m = 0;
    for (d_k = 1; d_k < d_k_end - 1; d_k++) /* note difference: -1 */ {
        d_pt = d_pts[d_k];
        d_me = d_met[d_k];
        for (i_k = m; i_k < i_nop; i_k++) {
            i_pt = i_pts[i_k];
            d = sq_dist (&d_pt, &i_pt);
            if (i_k < i_nop - 1) {
                i_me = i_met[i_k];
                if (d < LIMIT_LENGTH &&
                    abs (d_me.angle - i_me.angle) < M_PI_2) {
                    m = i_k;
                    ret += d;
                    break;
                } else {
                    /* Distance between each characteristic points and line */
                    r = i_me.a * d_pt.x + i_me.b * d_pt.y - i_me.e;
                    d = abs (i_me.a * d_pt.y - i_me.b * d_pt.x - i_me.c);
                    if (0 <= r && r <= i_me.d * i_me.d &&
                        d < LIMIT_LENGTH * i_me.d &&
                        abs (d_me.angle - i_me.angle) < M_PI_2) {
                        m = i_k;
                        ret += d;
                        break;
                    }
                }
            } else {
                if (d < LIMIT_LENGTH) {
                    m = i_k;
                    ret += d;
                    break;
                }
            }
        }
        if (i_k >= i_nop) {
            free (i_met);
            free (d_met);
            return -1;
        }
    }
    free (i_met);
    free (d_met);
    return d;
}

static GPtrArray *
get_candidates (TomoeStroke *input_stroke, GPtrArray *cands)
{
    GPtrArray     *rtn_cands;
    int            cand_index = 0;
    unsigned int   strk_index = 0;
    TomoeStroke    dict_stroke;
    int            i_nop = 0;    /* input stroke number of points */
    TomoePoint    *i_pts = NULL; /* input stroke points */
    tomoe_metric  *i_met = NULL; /* input stroke metrics */
    int            d_nop = 0;    /* dict stroke number of points */
    TomoePoint    *d_pts = NULL; /* dict stroke points */
    tomoe_metric  *d_met = NULL; /* dict stroke metrics */

    rtn_cands = g_ptr_array_new ();

    i_nop = input_stroke->point_num;
    i_pts = input_stroke->points;
    stroke_calculate_metrics (input_stroke, &i_met);

    for (cand_index = 0; cand_index < cands->len; cand_index++) {
        gboolean match_flag = FALSE;
        cand_priv *cand_p;
        GArray *tmp = NULL;
        TomoeChar *lttr;
        TomoeCandidate *cand;

        cand_p = g_ptr_array_index (cands, cand_index);
        tmp = _g_array_copy_int_value (cand_p->adapted_strokes);
        cand = TOMOE_CANDIDATE (cand_p->cand);
        lttr = tomoe_candidate_get_char (cand);

        for (strk_index = 0;
             strk_index < tomoe_char_get_glyph (lttr)->stroke_num;
             strk_index++) {
            int d1 = 0, d2 = 0;
            int d3 = 0, d4 = 0;
            int score1 = 0, score2 = 0;
            int score3 = 0;

            /* if the stroke index is already appended to, the value is ignored */
            if (_g_array_has_this_int_value (tmp, strk_index)) {
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
            tomoe_candidate_set_score (
                cand,
                tomoe_candidate_get_score (cand) + score3);
            if (d1 > LIMIT_LENGTH ||
                d2 > LIMIT_LENGTH ||
                abs (d_nop - i_nop) > 3) {
                free (d_met);
                continue;
            }

            d3 = sq_dist (&i_pts[0], &i_pts[1]);
            d4 = sq_dist (&d_pts[0], &d_pts[1]);
            /* threshold is (angle of bigining line) % 45[degree] (PI/4)*/
            if (d1 > LIMIT_LENGTH &&
                d2 > LIMIT_LENGTH &&
                abs (d_met[0].angle - i_met[0].angle) > M_PI_4) {
                free (d_met);
                continue;
            }

            /*
             * Distance and angle of each characteristic points:
             * (Compare handwriting data with dictionary data)
             */
            score1 = match_input_to_dict (input_stroke, &dict_stroke);
            if (score1 < 0) {
                free (d_met);
                tomoe_candidate_set_score (
                    cand,
                    tomoe_candidate_get_score (cand) * 2);
                continue;
            }
            tomoe_candidate_set_score (
                cand,
                tomoe_candidate_get_score (cand) + score1);

            /*
             * Distance and angle of each characteristic points:
             * (Compare dictionary data with handwriting data)
             */
            score2 = match_dict_to_input (&dict_stroke, input_stroke);
            if (score2 < 0) {
                free (d_met);
                tomoe_candidate_set_score (
                    cand,
                    tomoe_candidate_get_score (cand) * 2);
                continue;
            }
            tomoe_candidate_set_score (
                cand_p->cand,
                tomoe_candidate_get_score (cand) + score2);

            g_array_append_val (cand_p->adapted_strokes, strk_index);
            match_flag = TRUE;

            strk_index = tomoe_char_get_glyph (lttr)->stroke_num;

            free (d_met);
        }

        if (match_flag) {
            g_ptr_array_add (rtn_cands, cand_p);
        }
        g_array_free (tmp, TRUE);
    }

    free (i_met);

    return rtn_cands;
}

static int
match_stroke_num (TomoeDict* dict, int letter_index, int input_stroke_num, GArray *adapted)
{
    const GPtrArray *letters = tomoe_dict_get_letters(dict);
    int pj = 100;
    gint adapted_num;
    TomoeChar* chr = (TomoeChar*) g_ptr_array_index (letters, letter_index);
    int d_stroke_num = tomoe_char_get_glyph (chr)->stroke_num;

    if (!adapted)
        return -1;

    adapted_num = adapted->len;

    if (d_stroke_num - input_stroke_num >= 3) {
        int i, j;

        pj = 100;

        for (i = 0; i < adapted_num; i++) {
            j = g_array_index (adapted, gint, i);
            if (j - pj >= 3) {
                return -1;
            }
            pj = j;
        }
    }
    return pj;
}


static gboolean
_g_array_has_this_int_value (GArray *a, gint i)
{
    gint l;

    if (!a || a->len == 0) return FALSE;

    for (l = 0; l < a->len; l++) {
        if (g_array_index (a, gint, l) == i)
	        return TRUE;
    }
    return FALSE;
}

static GArray *
_g_array_copy_int_value (GArray *a)
{
    gint i;
    GArray *b = g_array_new (FALSE, FALSE, sizeof (gint));

    for (i = 0; i < a->len; i++) {
        g_array_append_val (b, g_array_index (a, gint, i));
    }
    return b;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
