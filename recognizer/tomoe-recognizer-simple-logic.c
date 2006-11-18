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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tomoe-char.h"
#include "tomoe-dict.h"
#include "tomoe-recognizer-simple-logic.h"

#define LIMIT_LENGTH ((300 * 0.25) * (300 * 0.25))

/* TODO remove from tomoe, replace by TomoeArray */
typedef struct _IntArray IntArray;

struct _IntArray
{
    int *p;
    int  len;
    int  ref_count;
};

/* TODO remove from tomoe, replace by TomoeArray */
typedef struct _PointerArray PointerArray;

struct _PointerArray
{
    void **p;
    int    len;
    int    ref_count;
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

static IntArray       *_int_array_new                  (void);
static IntArray       *_int_array_append_data          (IntArray *a, int i);
#if 0
static IntArray       *_int_array_ref                  (IntArray *a);
#endif
static IntArray       *_int_array_copy                 (IntArray *a);
static int             _int_array_find_data            (IntArray *a, int i);
static void            _int_array_unref                (IntArray *a);
static PointerArray   *_pointer_array_new              (void);
static PointerArray   *_pointer_array_append_data      (PointerArray *a, void *p);
static PointerArray   *_pointer_array_ref              (PointerArray *a);
#if 0
static int             _pointer_array_find_data        (PointerArray *a, void *p);
#endif
static void            _pointer_array_unref            (PointerArray *a);

static cand_priv      *cand_priv_new                   (TomoeChar*       character,
                                                        int              index);
static void            cand_priv_free                  (cand_priv       *cand_p,
                                                        TomoeBool        free_candidate);

static PointerArray   *get_candidates                  (TomoeDict *t_dict,
                                                        TomoeStroke*     input_stroke,
                                                        PointerArray    *cands);

static int             match_stroke_num                (TomoeDict *t_dict,
                                                        int              letter_index,
                                                        int              input_stroke_num,
                                                        IntArray        *adapted);


TomoeArray *
_tomoe_recognizer_simple_get_candidates (void *context, TomoeDict *dict, TomoeGlyph *input)
{
    /* TomoeRecognizerSimple *recognizer = context; */
    TomoeArray* matched = tomoe_array_new((tomoe_compare_fn)tomoe_candidate_compare,
                                          (tomoe_addref_fn)tomoe_candidate_add_ref,
                                          (tomoe_free_fn)tomoe_candidate_free);
    guint i, j;
    IntArray *matches = NULL;
    PointerArray *cands = NULL;
    PointerArray *first_cands = NULL;
    unsigned int letters_num = 0;
    const GPtrArray *letters = NULL;

    if (!input) return 0;
    if (!input->stroke_num) return 0;
    if (!dict) return 0;

    first_cands = _pointer_array_new ();
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
        cand = cand_priv_new(p, i);
        _pointer_array_append_data(first_cands, cand);
    }

    cands = first_cands;
    first_cands = _pointer_array_ref(first_cands);

    for (i = 0; i < (unsigned int)input->stroke_num; i++) {
        PointerArray *verbose_cands;

        verbose_cands = get_candidates(dict, &input->strokes[i], cands);
        _pointer_array_unref(cands);
        cands = verbose_cands;
    }

    matches = _int_array_new();
    for (i = 0; i < (unsigned int)cands->len; i++) {
        cand_priv *cand;
        IntArray *adapted;
        int pj;

        cand = cands->p[i];
        adapted = cand->adapted_strokes;
        pj = match_stroke_num(dict, cand->index, input->stroke_num, adapted);

        if (pj < 0)
            continue;

        if (pj != 0)
            cand->cand->score = cand->cand->score / pj;

        if (_int_array_find_data(matches, cand->index) < 0) {
            const TomoeChar *a = g_ptr_array_index (letters, cand->index);
            TomoeBool b = TRUE;

            for (j = 0; j < (unsigned int)matches->len; j++) {
                const TomoeChar *b = g_ptr_array_index (letters, matches->p[j]);
                if (!tomoe_char_compare(a, b)) {
                    b = FALSE;
                    break;
                }
            }
            if (b) {
                matches = _int_array_append_data(matches, cand->index);
            }
        }
    }

    if (cands->len > 0) {
        for (i = 0; i < (unsigned int)cands->len; i++) {
            int index = ((cand_priv *)cands->p[i])->index;

            if (_int_array_find_data(matches, index) >= 0) {
                TomoeCandidate* cand = tomoe_candidate_new();
                cand->character = tomoe_char_add_ref(((cand_priv*)cands->p[i])->cand->character);
                cand->score     = ((cand_priv *)cands->p[i])->cand->score;
                tomoe_array_append (matched, cand);
                tomoe_candidate_free (cand);
            }
        }
    }
    _int_array_unref (matches);

    tomoe_array_sort (matched);

    _pointer_array_unref (cands);

    for (i = 0; i < (unsigned int) first_cands->len; i++) {
        cand_priv_free (first_cands->p[i], TRUE);
    }

    _pointer_array_unref (first_cands);

    return matched;
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

static PointerArray *
get_candidates (TomoeDict *dict, TomoeStroke *input_stroke, PointerArray *cands)
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

    for (cand_index = 0; cand_index < cands->len; cand_index++) {
        TomoeBool match_flag = FALSE;
        IntArray *adapted = NULL;
        const GPtrArray *letters = tomoe_dict_get_letters(dict);

        cand = cands->p[cand_index];
        adapted = _int_array_copy (cand->adapted_strokes);
        lttr = g_ptr_array_index (letters, cand->index);

        for (strk_index = 0;
             strk_index < tomoe_char_get_glyph (lttr)->stroke_num;
             strk_index++) {
            int d1 = 0, d2 = 0;
            int d3 = 0, d4 = 0;
            int score1 = 0, score2 = 0;
            int score3 = 0;

            if (_int_array_find_data (adapted, strk_index) >= 0) {
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
                cand->cand->score = cand->cand->score * 2;
                continue;
            }
            cand->cand->score += score1;

            /*
             * Distance and angle of each characteristic points:
             * (Compare dictionary data with handwriting data)
             */
            score2 = match_dict_to_input (&dict_stroke, input_stroke);
            if (score2 < 0) {
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

        if (match_flag) {
            _pointer_array_append_data (rtn_cands, cand);
        }
        _int_array_unref (adapted);
    }

    free (i_met);

    return rtn_cands;
}

static int
match_stroke_num (TomoeDict* dict, int letter_index, int input_stroke_num, IntArray *adapted)
{
    const GPtrArray *letters = tomoe_dict_get_letters(dict);
    int pj = 100;
    int adapted_num;
    TomoeChar* chr = (TomoeChar*) g_ptr_array_index (letters, letter_index);
    int d_stroke_num = tomoe_char_get_glyph (chr)->stroke_num;

    if (!adapted)
        return -1;

    adapted_num = adapted->len;

    if (d_stroke_num - input_stroke_num >= 3) {
        int i, j;

        pj = 100;

        for (i = 0; i < adapted_num; i++) {
            j = adapted->p[i];
            if (j - pj >= 3) {
                return -1;
            }
            pj = j;
        }
    }
    return pj;
}

static IntArray *
_int_array_new (void)
{
    IntArray *a;

    a = calloc (sizeof (IntArray), 1);
    a->len = 0;
    a->p = NULL;
    a->ref_count = 1;

    return a;
}

static IntArray *
_int_array_append_data (IntArray *a, int i)
{
    if (!a)
        return NULL;
  
    a->len++;
    a->p = realloc (a->p, sizeof (int) * (a->len));
    a->p[a->len - 1] = i;

    return a;
}

static IntArray *
_int_array_copy (IntArray *a)
{
    int i;
    IntArray *ret;

    ret = _int_array_new ();

    ret->len = a->len;

    ret->p = calloc (ret->len, sizeof (int));
 
    for (i = 0; i < ret->len; i++) {
        ret->p[i] = a->p[i];
    }

    return ret;
}

#if 0
static IntArray *
_int_array_ref (IntArray *a)
{
    if (!a)
        return NULL;
    a->ref_count++;

    return a;
}
#endif
static void
_int_array_unref (IntArray *a)
{
    if (!a)
        return;
 
    a->ref_count--;
    if (a->ref_count == 0) {
        if (a->p)
            free (a->p);
        a->p = NULL;

        free (a);
    }
}

static int
_int_array_find_data (IntArray *a, int i)
{
    int l;

    if (!a || a->len == 0)
        return -1;
  
    for (l = 0; l < a->len; l++) {
        if (a->p[l] == i)
            return l;
    }

    return -1;
}

static PointerArray *
_pointer_array_new (void)
{
    PointerArray *a;

    a = calloc (sizeof (PointerArray), 1);
    a->len = 0;
    a->p = NULL;
    a->ref_count = 1;

    return a;
}

static PointerArray *
_pointer_array_append_data (PointerArray *a, void *p)
{
    if (!a)
        return NULL;
  
    a->len++;
    a->p = realloc (a->p, sizeof (void*) * (a->len));
    a->p[a->len - 1] = p;

    return a;
}

static PointerArray *
_pointer_array_ref (PointerArray *a)
{
    if (!a)
        return NULL;
    a->ref_count++;

    return a;
}

static void
_pointer_array_unref (PointerArray *a)
{
    if (!a)
        return;
 
    a->ref_count--;
    if (a->ref_count == 0) {
        if (a->p)
            free (a->p);
        a->p = NULL;

        free (a);
    }
}

#if 0
static int
_pointer_array_find_data (PointerArray *a, void *p)
{
    int l;
    if (!a)
      return -1;
  
    for (l = 0; l < a->len; l++) {
        if (a->p[l] == p)
            return l;
    }

    return -1;
}
#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/
