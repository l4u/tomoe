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

/*
 * This code is unfinished yet.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include "tomoe.h"

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

static dictionary *g_dict = NULL;

static void load_dictionaries        (void);
static void dictionary_free_contents (dictionary *dct);

static void candidate_init           (candidate *cand);
static void candidate_free_contents  (candidate *cand);

static void candidates_init             (candidates *cands);
static void candidates_free_contents    (candidates *cands);
static void candidates_append_candidate (candidates *cands, candidate *cand);

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
tomoe_get_matched (glyph *input, candidates **matched)
{
  int matched_num = 0;
  int i;
  candidates *cands = NULL;

  cands = (candidates *) calloc (1, sizeof(candidates));
  candidates_init(cands);
  
  if (!g_dict) return 0;
  
  for (i = 0; i < g_dict->letter_num; i++)
  {
    candidate *cand = NULL;
    /* check the number of stroke */
    if (input->stroke_num > g_dict->letters[i].c_glyph->stroke_num)
    {
      continue;
    }

    /* append a candidate to candidate list */
    cand = (candidate *) calloc (1, sizeof(candidate));
    candidate_init(cand);
    cand->letter = strdup (g_dict->letters[i].character);
    candidates_append_candidate(cands, cand);
  }

  *matched = cands;
  /* Not implemented yet*/
  return matched_num;
}

/* 
 * free matched characters 
 *
 * matched:
 *
 */
void
tomoe_free_matched (candidates *matched)
{
  /* Not implemented yet*/
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
  dictionary_free_contents(g_dict);
  /* Not implemented yet*/
}

/*
 * *******************
 *  utility functions
 * *******************
 */

static int
sq_len (int x, int y)
{
  return x * x + y * y;
}

static int
sq_dist (point *p, point *q)
{
  return sq_len (p->x - q->x, p->y - q->y);
}

static double
dabs (double a)
{
  return a > 0 ? a
    : -a;
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
  point *p = &strk->points[0];
  point *q;

  if (!strk) return 0;

  *met = (metric *) calloc (strk->point_num - 1, sizeof(metric));
 
  for (i = 0; i < strk->point_num - 1; i++, p++)
  {
    metric m = *met[i];
    q = p + 1;
    m.a = q->x - p->x;
    m.b = q->y - p->y;
    m.c = q->x * p->y - q->y * p->x;
    m.d = sqrt(m.a * m.a + m.b * m.b);
    m.e = m.a * p->x + m.b * p->y;
    m.angle = atan2(q->y - p->y, q->x - p->x);
  }
  
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
letter_free_contents(letter *lttr)
{
  int i;

  if (lttr->character != NULL)
  {
    free(lttr->character);
    lttr->character = NULL;
  }

  if (lttr->c_glyph != NULL)
  {
    for (i = 0; i < lttr->c_glyph->stroke_num; i++)
    {
      stroke_free_contents(&lttr->c_glyph->strokes[i]);
    }
    free(lttr->c_glyph->strokes);
    lttr->c_glyph->strokes = NULL;
  } 
}

/*
 * ***********************
 *  dictionary functions.
 * ***********************
 */

static void
dictionary_alloc_contents(dictionary *dct, int letter_num)
{
  dct->letter_num = letter_num;
  dct->letters = (letter *) calloc (letter_num, sizeof(letter));
}

static void
dictionary_free_contents(dictionary *dct)
{
  int i;

  if (!dct) return;

  if (dct->letters != NULL)
  {
    for (i = 0; i < dct->letter_num; i++)
    {
      letter_free_contents(&dct->letters[i]);
    }
    free(dct->letters);
    dct->letters = NULL;
  }
}

static void
dictionary_expand_to(dictionary *dct, int letter_num)
{
  letter *old_letters = dct->letters;
  int old_num = dct->letter_num;

  dct->letter_num = letter_num;
  dct->letters = (letter *)calloc(letter_num, sizeof(letter));
  if (old_letters != NULL)
  {
    memcpy(dct->letters, old_letters,
	   old_num * sizeof(letter));
    free(old_letters);
  }
}

/*
 * **********************
 *  candidate functions.
 * **********************
 */

static void
candidate_init(candidate *cand)
{
  cand->letter = NULL;
  cand->score = 0;
}

static void
candidate_free_contents(candidate *cand)
{
  if (cand->letter)
    free (cand->letter);
  cand->letter = NULL;
}

/*
 * ***********************
 *  candidates functions.
 * ***********************
 */

static void
candidates_init(candidates *cands)
{
  cands->candidate_num = 0;
  cands->candidates = NULL;
}

static void
candidates_free_contents(candidates *cands)
{
  int i;

  if (cands->candidates != NULL)
  {
    for (i = 0; i < cands->candidate_num; i++)
    {
      candidate_free_contents(&cands->candidates[i]);
    }
    free(cands->candidates);
    cands->candidates = NULL;
  }
}

static void
candidates_append_candidate (candidates *cands, candidate *cand)
{
  int old_cands_num = cands->candidate_num;
  candidate *old_cands = cands->candidates;
  cands->candidate_num++;
  cands->candidates =
    (candidate *)calloc(cands->candidate_num, sizeof(candidate));
  if (old_cands != NULL)
  {
    memcpy(cands->candidates, old_cands,
	   old_cands_num * sizeof(candidate));
    free(old_cands);
  }
  /* Ownership transfers from right hand side to left hand side. */
  cands->candidates[cands->candidate_num - 1] = *cand;
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

  FILE *fp = fopen(TOMOEDATADIR "/all.tdic", "r");
  while ((p = fgets(line_buf, LINE_BUF_SIZE, fp)) != NULL)
    {
      if (p[0] == '\n')
	{
	  continue;
	}
      ++letter_num;
      if (letter_num > g_dict->letter_num)
	{
	  dictionary_expand_to(g_dict,
			       g_dict->letter_num + DICT_LETTER_EXPAND_SIZE);
	}

      i = letter_num - 1;
      lttr = &g_dict->letters[i];
      p = strchr(p, '\n');
      if (p != NULL)
	{
	  *p = '\0';
	}
      lttr->character = strdup(line_buf);

      p = fgets(line_buf, LINE_BUF_SIZE, fp);
      if (p == NULL)
	{
	  break;
	}
      if (p[0] != ':')
	{
	  continue;
	}

      sscanf(p + 1, "%d", &stroke_num);

      letter_alloc_contents(lttr, stroke_num);

      for (j = 0; j < stroke_num; j++)
	{
	  strk = &lttr->c_glyph->strokes[j];
	  p = fgets(line_buf, LINE_BUF_SIZE, fp);
	  sscanf(p, "%d", &point_num);
	  p = strchr(p, ' ');
	  stroke_alloc_contents(strk, point_num);
	  for (k = 0; k < point_num; k++)
	    {
	      pnt = &strk->points[k];
	      sscanf(p, " (%d %d)", &pnt->x, &pnt->y);
	      p = strchr(p, ')') + 1;
	    }
	  
	  /* stroke_calculate_metrics(strk); */
	}

    }
  fclose(fp);

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
  point *i_pt = NULL;
  metric *i_me = NULL;
  point *d_pt = NULL;
  metric *d_me = NULL;
  int r = 0;

  i_nop = input_stroke->point_num;
  i_pts = input_stroke->points;
//  i_met = input_stroke->metrics;
  d_nop = dict_stroke->point_num;
  d_pts = dict_stroke->points;
//  d_met = dict_stroke->metrics;

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
      i_pt = &i_pts[i_k];
      i_me = &i_met[i_k];
      for (d_k = m; d_k < d_nop; d_k++)
	{
	  d_pt = &d_pts[d_k];
	  if (d_k < d_nop - 1)
	    {
	      d_me = &d_met[d_k];
	      if (sq_dist(i_pt, d_pt) < LIMIT_LENGTH &&
		  dabs(i_me->angle - d_me->angle) < M_PI_2)
		{
		  m = d_k;
		  break;
		}
	      else
		{
		  /* 各特徴点と線分との距離 */
		  r = d_me->a * i_pt->x + d_me->b * i_pt->y - d_me->e;
		  if (0 <= r && r <= d_me->d * d_me->d &&
		      dabs(d_me->a * i_pt->y - d_me->b * i_pt->x - d_me->c) <
		      LIMIT_LENGTH * d_me->d &&
		      dabs(i_me->angle - d_me->angle) < M_PI_2)
		    {
		      m = d_k;
		      break;
		    }
		}
	    }
	  else
	    {
	      if (sq_dist(i_pt, d_pt) < LIMIT_LENGTH)
		{
		  m = d_k;
		  break;
		}
	    }
	}
      if (d_k >= d_nop)
	{
	  return FALSE;
	}
    }
  return TRUE;
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
  point *d_pt = NULL;
  metric *d_me = NULL;
  point *i_pt = NULL;
  metric *i_me = NULL;
  int r = 0;

  d_nop = dict_stroke->point_num;
  d_pts = dict_stroke->points;
  //d_met = dict_stroke->metrics;
  i_nop = input_stroke->point_num;
  i_pts = input_stroke->points;
  //i_met = input_stroke->metrics;

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
      d_pt = &d_pts[d_k];
      d_me = &d_met[d_k];
      for (i_k = m; i_k < i_nop; i_k++)
	{
	  i_pt = &i_pts[i_k];
	  if (i_k < i_nop - 1)
	    {
	      i_me = &i_met[i_k];
	      if (sq_dist(d_pt, i_pt) < LIMIT_LENGTH &&
		  dabs(d_me->angle - i_me->angle) < M_PI_2)
		{
		  m = i_k;
		  break;
		}
	      else
		{
		  /* 各特徴点と線分との距離 */
		  r = i_me->a * d_pt->x + i_me->b * d_pt->y - i_me->e;
		  if (0 <= r && r <= i_me->d * i_me->d &&
		      dabs(i_me->a * d_pt->y - i_me->b * d_pt->x - i_me->c) <
		      LIMIT_LENGTH * i_me->d &&
		      dabs(d_me->angle - i_me->angle) < M_PI_2)
		    {
		      m = i_k;
		      break;
		    }
		}
	    }
	  else
	    {
	      if (sq_dist(d_pt, i_pt) < LIMIT_LENGTH)
		{
		  m = i_k;
		  break;
		}
	    }
	}
      if (i_k >= i_nop)
	{
	  return FALSE;
	}
    }
  return TRUE;
}

#if 0
static candidates
get_candidates(stroke *input_stroke, candidates cands)
{
  candidates rtn_cands;
  //int_array adapted;
  //int_array_array cache;
  candidate *cand = NULL;
  int cand_index = 0;
  int letter_index = 0;
  letter *lttr = NULL;
  int strk_index = 0;
  stroke *dict_stroke = NULL;
  char *current_char = NULL;
  int i_nop = 0;        /* input stroke number of points */
  point *i_pts = NULL;  /* input stroke points */
  metric *i_met = NULL; /* input stroke metrics */
  int d_nop = 0;        /* dict stroke number of points */
  point *d_pts = NULL;  /* dict stroke points */
  metric *d_met = NULL; /* dict stroke metrics */


  candidates_init(&rtn_cands);
  //int_array_init(&adapted);
  //int_array_array_alloc_contents(&cache, dict.letter_num);

  i_nop = input_stroke->point_num;
  i_pts = input_stroke->points;
  //i_met = input_stroke->metrics;

  for (cand_index = 0; cand_index < cands.candidate_num; cand_index++)
    {
      cand = &cands.candidates[cand_index];

      letter_index = cand->letter_index;

      int_array_free_contents(&adapted);
      int_array_copy(&cand->adapted_stroke_indexes, &adapted);

      lttr = &dict.letters[letter_index];
      current_char = lttr->character;

      for (strk_index = 0; strk_index < lttr->stroke_num; strk_index++)
	{
	  dict_stroke = &lttr->strokes[strk_index];
	  if (int_array_contains(&adapted, strk_index))
	    {
	      continue;
	    }

	  if (cache.data[letter_index].data == NULL)
	    {
	      int_array_alloc_contents(&cache.data[letter_index],
				       lttr->stroke_num);
	    }

	  if (cache.data[letter_index].data[strk_index] == 1)
	    {
	      int_array_add(&cand->adapted_stroke_indexes, strk_index);
	      candidates_add_deep_copy(&rtn_cands, cand);
	      break;
	    }
	  else if (cache.data[letter_index].data[strk_index] == -1)
	    {
	      continue;
	    }
	  cache.data[letter_index].data[strk_index] = -1;

	  d_nop = dict_stroke->point_num;
	  d_pts = dict_stroke->points;
	  d_met = dict_stroke->metrics;

	  /* 始点・終点との距離, 特徴点の数 */
	  if (sq_dist(&i_pts[0], &d_pts[0]) > LIMIT_LENGTH ||
	      sq_dist(&i_pts[i_nop - 1], &d_pts[d_nop - 1]) > LIMIT_LENGTH ||
	      abs(d_nop - i_nop) > 3)
	    {
	      continue;
	    }

	  /* 始線の角度 % 45 度 (PI/4) がしきい値 */
	  if (sq_dist(&i_pts[0], &i_pts[1]) > LIMIT_LENGTH &&
	      sq_dist(&d_pts[0], &d_pts[1]) > LIMIT_LENGTH &&
	      dabs(d_met[0].angle - i_met[0].angle) > M_PI_4)
	    {
	      continue;
	    }
	    
	  /* 各特徴点の距離と角度: (手書き文字を辞書と比較) */
	  if (!match_input_to_dict(input_stroke, dict_stroke))
	    {
	      continue;
	    }

	  /* 各特徴点の距離と角度: (辞書を手書き文字と比較) */
	  if (!match_dict_to_input(dict_stroke, input_stroke))
	    {
	      continue;
	    }

	  cache.data[letter_index].data[strk_index] = 1;
	  int_array_add(&cand->adapted_stroke_indexes, strk_index);
	  candidates_add_deep_copy(&rtn_cands, cand);
	}
    }
  int_array_free_contents(&adapted);

  int_array_array_free_contents(&cache);

  return rtn_cands;
}

static int
match_stroke_num(int letter_index, int_array *adapted)
{
  int pj;
  int i;
  int j;
  int i_stroke_num = input.stroke_num;
  int d_stroke_num = dict.letters[letter_index].stroke_num;
  if (d_stroke_num - i_stroke_num >= 3)
    {
      pj = 100;
      for (i = 0; i < adapted->num; i++) {
	j = adapted->data[i];
	if (j - pj >= 3)
	  {
	    return FALSE;
	  }
	pj = j;
      }
    }
  return TRUE;
}


static candidates
match_char()
{
  candidates cands;
  candidates verbose_cands;
  candidate cand;
  candidate *verbose_cand;
  int_array *verbose_adapted;
  int_array_array adapted_array;
  int i = 0;
  int j = 0;
  int stroke_index = 0;
  int verbose_letter_index = 0;
  int pre_char = 0;
  stroke *input_stroke = NULL;

  candidates_init(&cands);
  int_array_array_init(&adapted_array);

  for (i = 0; i < dict.letter_num; i++)
    {
      if (input.stroke_num <= dict.letters[i].stroke_num)
	{
	  candidate_init(&cand);
	  cand.letter_index = i;
	  candidates_add_owned(&cands, &cand);
	}
    }

  for (stroke_index = 0; stroke_index < input.stroke_num; stroke_index++)
    {
      input_stroke = &input.strokes[stroke_index];
      verbose_cands = get_candidates(input_stroke, cands);

      candidates_free_contents(&cands);
      candidates_init(&cands);

      int_array_array_init(&adapted_array);

      for (j = 0; j < verbose_cands.candidate_num; j++)
	{
	  verbose_cand = &verbose_cands.candidates[j];
	  verbose_letter_index = verbose_cand->letter_index;
	  verbose_adapted = &verbose_cand->adapted_stroke_indexes;
	  int_array_sort(verbose_adapted);

	  if (pre_char != verbose_letter_index ||
	      !int_array_array_contains(&adapted_array, verbose_adapted))
	    {
	      int_array_array_add_copy(&adapted_array, verbose_adapted);
	      candidates_add_deep_copy(&cands, verbose_cand);
	    }
	  pre_char = verbose_letter_index;
	}

      int_array_array_free_contents(&adapted_array);

      candidates_free_contents(&verbose_cands);
    }
  return cands;
}


static int
compare_letter_indexes_by_stroke_num(const void *a, const void *b)
{
  const int *index_a = (const int *) a;
  const int *index_b = (const int *) b;
  int stroke_num_a = dict.letters[*index_a].stroke_num;
  int stroke_num_b = dict.letters[*index_b].stroke_num;
  return stroke_num_a > stroke_num_b ? 1
    : stroke_num_a < stroke_num_b ? -1
    : 0;
}


static void
sort_letter_indexes_by_stroke_num(int_array *letter_indexes)
{
  qsort(letter_indexes->data, letter_indexes->num, sizeof(int),
	compare_letter_indexes_by_stroke_num);
}

static int_array
get_matched_char_index()
{
  int i;
  int_array matched;
  candidate *cand = NULL;
  int pre_char = -1;
  int letter_index = 0;
  int_array *adapted = NULL;
  candidates cands = match_char();

  int_array_init(&matched);
  for (i = 0; i < cands.candidate_num; i++)
    {
      cand = &cands.candidates[i];
      letter_index = cand->letter_index;
      adapted = &cand->adapted_stroke_indexes;
      if (!match_stroke_num(letter_index, adapted))
	{
	  pre_char = letter_index;
	  continue;
	}

      if (pre_char != letter_index &&
	  !int_array_contains(&matched, letter_index))
	{
	  int_array_add(&matched, letter_index);
	}
    }
  sort_letter_indexes_by_stroke_num(&matched);

  return matched;
}
#endif

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

/*
vi:ts=2:ai:expandtab
*/
