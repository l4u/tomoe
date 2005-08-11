/*
 *  Copyright (C) 2000 - 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
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
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include "tomoe.h"
#include "array.h"

#define LINE_BUF_SIZE 4096
#define DICT_LETTER_INITIAL_SIZE 3049
#define DICT_LETTER_EXPAND_SIZE 10
#define LIMIT_LENGTH ((300 * 0.25) * (300 * 0.25))

typedef struct _letter letter;
typedef struct _dictionary dictionary;
typedef struct _metric metric;

struct _dictionary
{
  char *file_name;
  char *dictionary_name;
  char *encoding;
  char *lang;
  int letter_num;
  letter *letters;
};

struct _letter
{
  char *character;
  glyph *c_glyph;
};

struct _metric
{
  int a;
  int b;
  int c;
  double d;
  int e;
  double angle;
};

typedef struct _candidate_private candidate_private;

struct _candidate_private
{
  candidate *cand;
  int        index;
  int_array *adapted_strokes;
};

static dictionary *g_dict = NULL;

static void load_dictionaries           (void);
static void dictionary_free_contents    (dictionary *dct);

static candidate_private *candidate_private_new  (const char *letter, int index);
static void               candidate_private_free (candidate_private *cand_p, tomoe_bool free_candidate);

static pointer_array *get_candidates    (stroke *input_stroke, pointer_array *cands);

static int match_stroke_num             (int letter_index, int input_stroke_num, int_array *adapted);
static void candidate_sort_by_score (candidate **cands, int length);
/* 
 * Initialize tomoe 
 */
void
tomoe_init (void)
{
  /* load all available dictionaries */
  load_dictionaries ();
  /* Not implemented yet*/
}

/* 
 * get matched characters 
 *
 * matched:
 * Return value: the number of matched characters
 *
 */
int
tomoe_get_matched (glyph *input, candidate ***matched)
{
  int matched_num = 0;
  int i, j; 
  int_array *matches = NULL;
  pointer_array *cands = NULL;
  pointer_array *first_cands = NULL;
  candidate **ret = NULL;
  
  if (!input) return 0;
  if (!input->stroke_num) return 0;
  if (!g_dict) return 0;

  first_cands = pointer_array_new ();
  
  for (i = 0; i < g_dict->letter_num; i++)
  {
    candidate_private *cand = NULL;
    /* check the number of stroke */
    if (input->stroke_num > g_dict->letters[i].c_glyph->stroke_num)
    {
      continue;
    }

    /* append a candidate to candidate list */
    cand = candidate_private_new ((const char*) g_dict->letters[i].character, i);
    pointer_array_append_data (first_cands, cand);
  }

  cands = first_cands;
  first_cands = pointer_array_ref (first_cands);

  for (i = 0; i < input->stroke_num; i++)
  {
    pointer_array *verbose_cands;
    verbose_cands = get_candidates (&input->strokes[i], cands);
    pointer_array_unref (cands);
    cands = verbose_cands;
  }

  matches = int_array_new ();
  for (i = 0; i < cands->len; i++)
  {
    candidate_private *cand;
    int_array *adapted;
    cand = cands->p[i];
    adapted = cand->adapted_strokes;
    int pj;

    pj = match_stroke_num(cand->index, input->stroke_num, adapted);

    if (pj < 0)
    {
      continue;
    }

    cand->cand->score = cand->cand->score * 100 / pj;

    if (int_array_find_data (matches, cand->index) < 0)
    {
      tomoe_bool b = TRUE;
      for (j = 0; j < matches->len; j++)
      {
        if (!strcmp (g_dict->letters[cand->index].character, 
                     g_dict->letters[matches->p[j]].character))
        {
          b = FALSE;
          break;
        }
      }
      if (b)
      {
        matches = int_array_append_data (matches, cand->index);
      }
    }
  }
  matched_num = matches->len;

  if (cands->len > 0)
  {
    int pos = 0;
    ret = (candidate **) calloc (sizeof (candidate*), matched_num);
    for (i = 0; i < cands->len; i++)
    {
      int index = ((candidate_private *)cands->p[i])->index;
      if (int_array_find_data (matches, index) >= 0)
      {
        candidate *cand;
        cand = (candidate *) calloc (sizeof (candidate), 1);
        cand->letter = ((candidate_private *)cands->p[i])->cand->letter;
        cand->score = ((candidate_private *)cands->p[i])->cand->score;
        ret[pos] = cand;
        pos++;
      }
    }
    matched_num = pos;
  }
  int_array_unref (matches);

  *matched = ret;

  candidate_sort_by_score (ret, matched_num);

  pointer_array_unref (cands);
  
  for (i = 0; i < first_cands->len; i++)
  {
    candidate_private_free (first_cands->p[i], TRUE);
  }
  pointer_array_unref (first_cands);
  return matched_num;
}

/* 
 * free matched characters 
 *
 * matched:
 *
 */
void
tomoe_free_matched (candidate **matched, int len)
{
  int i;
  for (i = 0; i< len; i++)
  {
    free (matched[i]);
  }

  free (matched);
}

/* register characters with stroke */
tomoe_bool
tomoe_data_register (glyph *input, char *data)
{
  tomoe_bool ret = FALSE;
  /* Not implemented yet*/
  return ret;
}

/* finalize tomoe */
void 
tomoe_term (void)
{
  dictionary_free_contents (g_dict);

  free (g_dict);
}

/*
 * *******************
 *  utility functions
 * *******************
 */

#define SQUARE_LENGTH(x, y) ((x) * (x) + (y) * (y))

static int
sq_dist (point *p, point *q)
{
  return SQUARE_LENGTH (p->x - q->x, p->y - q->y);
}

/*
 * *******************
 *  stroke functions.
 * *******************
 */

static void
stroke_alloc_contents (stroke *strk, int point_num)
{
  strk->point_num = point_num;
  strk->points = (point *) calloc (point_num, sizeof(point));
}

static void
stroke_free_contents (stroke *strk)
{
  if (strk->points != NULL)
  {
    free (strk->points);
    strk->points = NULL;
  }
}

static int 
stroke_calculate_metrics (stroke *strk, metric **met)
{
  int i = 0;
  point p;
  point q;
  metric *m;

  if (!strk) return 0;

  m = (metric *) calloc (strk->point_num - 1, sizeof(metric));
 
  for (i = 0; i < strk->point_num - 1; i++)
  {
    p = strk->points[i];
    q = strk->points[i + 1];
    m[i].a = q.x - p.x;
    m[i].b = q.y - p.y;
    m[i].c = q.x * p.y - q.y * p.x;
    m[i].d = sqrt(m[i].a * m[i].a + m[i].b * m[i].b);
    m[i].e = m[i].a * p.x + m[i].b * p.y;
    m[i].angle = atan2 (q.y - p.y, q.x - p.x);
  }
 
  *met = m;
  return strk->point_num - 1;
}

/*
 * *******************
 *  letter functions.
 * *******************
 */

static void
letter_alloc_contents (letter *lttr, int stroke_num)
{
  lttr->c_glyph = (glyph *) calloc (1, sizeof(glyph));
  lttr->c_glyph->stroke_num = stroke_num;
  lttr->c_glyph->strokes = (stroke *) calloc (stroke_num, sizeof(stroke));
}

static void
letter_free_contents (letter *lttr)
{
  int i;

  if (lttr->character != NULL)
  {
    free (lttr->character);
    lttr->character = NULL;
  }

  if (lttr->c_glyph != NULL)
  {
    for (i = 0; i < lttr->c_glyph->stroke_num; i++)
    {
      stroke_free_contents (&lttr->c_glyph->strokes[i]);
    }
    free (lttr->c_glyph->strokes);
    lttr->c_glyph->strokes = NULL;
    free (lttr->c_glyph);
    lttr->c_glyph = NULL;
  } 
}

/*
 * ***********************
 *  dictionary functions.
 * ***********************
 */

static void
dictionary_alloc_contents (dictionary *dct, int letter_num)
{
  dct->letter_num = letter_num;
  dct->letters = (letter *) calloc (letter_num, sizeof(letter));
}

static void
dictionary_free_contents (dictionary *dct)
{
  int i;

  if (!dct) return;

  if (dct->letters != NULL)
  {
    for (i = 0; i < dct->letter_num; i++)
    {
      letter_free_contents (&dct->letters[i]);
    }
    free (dct->letters);
    dct->letters = NULL;
  }
}

static void
dictionary_expand_to (dictionary *dct, int letter_num)
{
  dct->letter_num = letter_num;
  dct->letters = (letter *) realloc (dct->letters, letter_num * sizeof(letter));
}

/*
 * **********************
 *  candidate private_functions.
 * **********************
 */

static candidate_private *
candidate_private_new (const char *letter, int index)
{
  candidate_private *cand_p;

  cand_p = (candidate_private *) calloc (sizeof (candidate_private), 1);
  cand_p->cand = (candidate *) calloc (sizeof (candidate), 1);
  cand_p->cand->letter = letter;
  cand_p->cand->score = 0;
  cand_p->index = index;
  cand_p->adapted_strokes = int_array_new ();

  return cand_p;
}

static void
candidate_private_free (candidate_private *cand_p, tomoe_bool free_candidate)
{
  if (!cand_p)
    return;
  if (cand_p->adapted_strokes)
    int_array_unref (cand_p->adapted_strokes);
  cand_p->adapted_strokes = NULL;

  if (free_candidate)
    free (cand_p->cand);
  cand_p->cand = NULL;

  free (cand_p);
}

static int
compare_candidate_score (const void *a, const void *b)
{
  const candidate **cand_a = (const candidate **) a;
  const candidate **cand_b = (const candidate **) b;
  int score_a = cand_a[0]->score;
  int score_b = cand_b[0]->score;
  return score_a > score_b ? 1
    : score_a < score_b ? -1
    : 0;
}

static void
candidate_sort_by_score (candidate **cands, int length)
{
  qsort(cands, length, sizeof(void*),
      	compare_candidate_score);
}

/*
 * ***********************
 *  data load functions.
 * ***********************
 */
static void
load_dictionaries (void)
{
  char *p = NULL;
  int letter_num = 0;
  int stroke_num = 0;
  int point_num = 0;
  int i = 0;
  int j = 0;
  int k = 0;
  letter *lttr;
  stroke *strk = NULL;
  point *pnt = NULL;
  char line_buf[LINE_BUF_SIZE];

  if (g_dict) return;

  g_dict = calloc (1, sizeof(dictionary));
  dictionary_alloc_contents (g_dict, DICT_LETTER_INITIAL_SIZE);

  FILE *fp = fopen (TOMOEDATADIR "/all.tdic", "r");
  while ((p = fgets (line_buf, LINE_BUF_SIZE, fp)) != NULL)
  {
    if (p[0] == '\n')
	  {
	    continue;
	  }
    ++letter_num;
    if (letter_num > g_dict->letter_num)
	  {
	    dictionary_expand_to (g_dict,
			                      g_dict->letter_num + DICT_LETTER_EXPAND_SIZE);
	  }

    i = letter_num - 1;
    lttr = &g_dict->letters[i];
    p = strchr (p, '\n');
    if (p != NULL)
	  {
	    *p = '\0';
	  }
    lttr->character = strdup (line_buf);

    p = fgets (line_buf, LINE_BUF_SIZE, fp);
    if (p == NULL)
	  {
	    break;
	  }
    if (p[0] != ':')
	  {
	    continue;
	  }

    sscanf (p + 1, "%d", &stroke_num);

    letter_alloc_contents (lttr, stroke_num);

    for (j = 0; j < stroke_num; j++)
	  {
	    strk = &lttr->c_glyph->strokes[j];
	    p = fgets (line_buf, LINE_BUF_SIZE, fp);
	    sscanf (p, "%d", &point_num);
	    p = strchr (p, ' ');
	    stroke_alloc_contents (strk, point_num);
	    for (k = 0; k < point_num; k++)
	    {
	      pnt = &strk->points[k];
	      sscanf (p, " (%d %d)", &pnt->x, &pnt->y);
	      p = strchr (p, ')') + 1;
	    }
	  
	    /*stroke_calculate_metrics(strk);*/
	  }

  }
  fclose (fp);

  if (letter_num < g_dict->letter_num)
  {
    g_dict->letter_num = letter_num;
  }
}

/*
 * ************************************
 *  handwriting recognition functions.
 * ************************************
 */

static int
match_input_to_dict(stroke *input_stroke, stroke *dict_stroke)
{
  int i_nop = 0;        /* input stroke number of points */
  point *i_pts = NULL;  /* input stroke points */
  metric *i_met = NULL; /* input stroke metrics */
  int d_nop = 0;        /* dict stroke number of points */
  point *d_pts = NULL;  /* dict stroke points */
  metric *d_met = NULL; /* dict stroke metrics */
  int i_k_end = 0;
  int i_k = 0;
  int d_k = 0;
  int m = 0;
  point i_pt;
  metric i_me;
  point d_pt;
  metric d_me;
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
   * if the length between last point and second last point is lesser than LIMIT_LENGTH,
   * the last itinerary assumes "hane".
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
		        abs(i_me.angle - d_me.angle) < M_PI_2)
		    {
		      m = d_k;
          ret += d;
		      break;
		    }
	      else
		    {
		      /* 各特徴点と線分との距離 */
		      r = d_me.a * i_pt.x + d_me.b * i_pt.y - d_me.e;
          d = abs(d_me.a * i_pt.y - d_me.b * i_pt.x - d_me.c);
		      if (0 <= r && r <= d_me.d * d_me.d &&
		          d < LIMIT_LENGTH * d_me.d &&
		          abs(i_me.angle - d_me.angle) < M_PI_2)
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
match_dict_to_input(stroke *dict_stroke, stroke *input_stroke)
{
  int d_nop = 0;        /* dict stroke number of points */
  point *d_pts = NULL;  /* dict stroke points */
  metric *d_met = NULL; /* dict stroke metrics */
  int i_nop = 0;        /* input stroke number of points */
  point *i_pts = NULL;  /* input stroke points */
  metric *i_met = NULL; /* input stroke metrics */
  int d_k_end = 0;
  int d_k = 0;
  int i_k = 0;
  int m = 0;
  point d_pt;
  metric d_me;
  point i_pt;
  metric i_me;
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
   * if the length between last point and second last point is lesser than LIMIT_LENGTH,
   * the last itineraryassumes "hane".
   */
  if (sq_dist(&d_pts[d_nop - 1], &d_pts[d_nop - 2]) < LIMIT_LENGTH)
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
	    d = sq_dist(&d_pt, &i_pt);
	    if (i_k < i_nop - 1)
	    {
	      i_me = i_met[i_k];
	      if (d < LIMIT_LENGTH &&
		        abs(d_me.angle - i_me.angle) < M_PI_2)
		    {
		       m = i_k;
           ret += d;
		       break;
		    }
	      else
		    {
		      /* 各特徴点と線分との距離 */
		      r = i_me.a * d_pt.x + i_me.b * d_pt.y - i_me.e;
          d = abs(i_me.a * d_pt.y - i_me.b * d_pt.x - i_me.c);
		      if (0 <= r && r <= i_me.d * i_me.d &&
		          d < LIMIT_LENGTH * i_me.d &&
		          abs(d_me.angle - i_me.angle) < M_PI_2)
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
get_candidates(stroke *input_stroke, pointer_array *cands)
{
  pointer_array *rtn_cands;
  candidate_private *cand;
  int cand_index = 0;
  letter lttr;
  int strk_index = 0;
  stroke dict_stroke;
  int i_nop = 0;        /* input stroke number of points */
  point *i_pts = NULL;  /* input stroke points */
  metric *i_met = NULL; /* input stroke metrics */
  int d_nop = 0;        /* dict stroke number of points */
  point *d_pts = NULL;  /* dict stroke points */
  metric *d_met = NULL; /* dict stroke metrics */

  rtn_cands = pointer_array_new ();

  i_nop = input_stroke->point_num;
  i_pts = input_stroke->points;
  stroke_calculate_metrics (input_stroke, &i_met);

  for (cand_index = 0; cand_index < cands->len; cand_index++)
  {
    tomoe_bool match_flag;
    match_flag = FALSE;
    int_array *adapted = NULL;
    cand = cands->p[cand_index];

    adapted = int_array_copy (cand->adapted_strokes);

    lttr = g_dict->letters[cand->index];

    for (strk_index = 0; strk_index < lttr.c_glyph->stroke_num; strk_index++)
	  {
      int d1 = 0, d2 = 0;
      int d3 = 0, d4 = 0;
      int score1 = 0, score2 = 0;
      int score3 = 0;
      if (int_array_find_data (adapted, strk_index) >= 0)
      {
	        continue;
      }

	    dict_stroke = lttr.c_glyph->strokes[strk_index];
	    d_nop = dict_stroke.point_num;
	    d_pts = dict_stroke.points;
      stroke_calculate_metrics (&dict_stroke, &d_met);

	    /* 始点・終点との距離, 特徴点の数 */
      d1 = sq_dist(&i_pts[0], &d_pts[0]);
      d2 = sq_dist(&i_pts[i_nop - 1], &d_pts[d_nop - 1]);
      score3 = (d1 + d2);
      cand->cand->score += score3;
	    if (d1 > LIMIT_LENGTH ||
	        d2 > LIMIT_LENGTH ||
	        abs(d_nop - i_nop) > 3)
	    {
        free (d_met);
	      continue;
	    }

      d3 = sq_dist(&i_pts[0], &i_pts[1]);
      d4 = sq_dist(&d_pts[0], &d_pts[1]);
	    /* 始線の角度 % 45 度 (PI/4) がしきい値 */
      if (d1 > LIMIT_LENGTH &&
	        d2 > LIMIT_LENGTH &&
	        abs(d_met[0].angle - i_met[0].angle) > M_PI_4)
	    {
        free (d_met);
	      continue;
	    }

	    /* 各特徴点の距離と角度: (手書き文字を辞書と比較) */
      score1 = match_input_to_dict(input_stroke, &dict_stroke);
	    if (score1 < 0)
	    {
        free (d_met);
        cand->cand->score = cand->cand->score * 2;
	      continue;
	    }
      cand->cand->score += score1;

	    /* 各特徴点の距離と角度: (辞書を手書き文字と比較) */
      score2 = match_dict_to_input(&dict_stroke, input_stroke);
	    if (score2 < 0)
	    {
        free (d_met);
        cand->cand->score = cand->cand->score * 2;
	      continue;
	    }
      cand->cand->score += score2;

      int_array_append_data (cand->adapted_strokes, strk_index);
      match_flag = TRUE;

      strk_index = lttr.c_glyph->stroke_num;

      free (d_met);
	  }

    if (match_flag)
    {
	    pointer_array_append_data (rtn_cands, cand);
    }
    int_array_unref (adapted);
  }

  free (i_met);

  return rtn_cands;
}

static int
match_stroke_num(int letter_index, int input_stroke_num, int_array *adapted)
{
  int pj = 100;
  int i;
  int j;
  int adapted_num;
  int d_stroke_num = g_dict->letters[letter_index].c_glyph->stroke_num;

  if (!adapted)
    return -1;

  adapted_num = adapted->len;

  if (d_stroke_num - input_stroke_num >= 3)
  {
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
tomoe_glyph_free (glyph *g)
{
  int i;
  for (i = 0; i < g->stroke_num; i++)
  {
    stroke_free_contents (&g->strokes[i]);
  }
  if (g->strokes)
    free (g->strokes);
  g->strokes = NULL;

  free (g);
}

#if 0
static void
timeval_minus(struct timeval *tv1, struct timeval *tv2, struct timeval *diff)
{
  diff->tv_sec = tv1->tv_sec - tv2->tv_sec;
  diff->tv_usec = tv1->tv_usec - tv2->tv_usec;
  if (diff->tv_usec < 0)
    {
      diff->tv_usec += 1000000;
      diff->tv_sec--;
    }
}
#endif
/*
vi:ts=2:nowrap:ai:expandtab
*/
