/*
 *  Copyright (C) 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
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

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define LINE_BUF_SIZE 4096
#define DICT_LETTER_INITIAL_SIZE 3049
#define LIMIT_LENGTH ((300 * 0.25) * (300 * 0.25))

typedef struct dictionary_ dictionary;
typedef struct letter_ letter;
typedef struct stroke_ stroke;
typedef struct point_ point;
typedef struct metric_ metric;

typedef struct candidates_ candidates;
typedef struct candidate_ candidate;
typedef struct int_array_ int_array;
typedef struct int_array_array_ int_array_array;

struct dictionary_
{
  int letter_num;
  letter *letters;
};

struct letter_
{
  char *character;
  int stroke_num;
  stroke *strokes;
};

struct stroke_
{
  int point_num;
  point *points;
  metric *metrics;
};

struct point_
{
  int x;
  int y;
};

struct metric_
{
  int a;
  int b;
  int c;
  double d;
  int e;
  double angle;
};

struct int_array_
{
  int num;
  int *data;
};

struct int_array_array_
{
  int num;
  int_array *data;
};

struct candidate_
{
  int letter_index;
  int_array adapted_stroke_indexes;
};

struct candidates_
{
  int candidate_num;
  candidate *candidates;
};

/*
 * ********************
 *  global variables.  
 * ********************
 */

static dictionary dict;
static letter input;
static char line_buf[LINE_BUF_SIZE];
static int verbose = FALSE;

/*
 * *******************
 *  utility functions
 * *******************
 */

static int
sq_len(int x, int y)
{
  return x * x + y * y;
}

static int
sq_dist(point *p, point *q)
{
  return sq_len(p->x - q->x, p->y - q->y);
}

static double
dabs(double a)
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
stroke_alloc_contents(stroke *strk, int point_num)
{
  strk->point_num = point_num;
  strk->points = (point *)calloc(point_num, sizeof(point));
  strk->metrics = (metric *)calloc(point_num - 1, sizeof(metric));
}

static void
stroke_calculate_metrics(stroke *strk)
{
  int i = 0;
  point *p = &strk->points[0];
  point *q;
  metric *m = &strk->metrics[0];
  for (i = 0; i < strk->point_num - 1; i++, p++, m++)
    {
      q = p + 1;
      m->a = q->x - p->x;
      m->b = q->y - p->y;
      m->c = q->y * p->x - q->x * p->y;
      m->d = sqrt(m->a * m->a + m->b * m->b);
      m->e = m->a * p->x + m->b * p->y;
      m->angle = atan2(q->y - p->y, q->x - p->x);
    }
}

static void
stroke_free_contents(stroke *strk)
{
  if (strk->points != NULL)
    {
      free(strk->points);
      strk->points = NULL;
    }
  if (strk->metrics != NULL)
    {
      free(strk->metrics);
      strk->metrics = NULL;
    }
}

/*
 * *******************
 *  letter functions.
 * *******************
 */

static void
letter_alloc_contents(letter *lttr, int stroke_num)
{
  lttr->stroke_num = stroke_num;
  lttr->strokes = (stroke *)calloc(stroke_num, sizeof(stroke));
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
  if (lttr->strokes != NULL)
    {
      for (i = 0; i < lttr->stroke_num; i++)
	{
	  stroke_free_contents(&lttr->strokes[i]);
	}
      free(lttr->strokes);
      lttr->strokes = NULL;
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
  dct->letters = (letter *)calloc(letter_num, sizeof(letter));
}

static void
dictionary_free_contents(dictionary *dct)
{
  int i;

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


/*
 * **********************
 *  int_array functions.
 * **********************
 */

static void
int_array_init(int_array *array)
{
  array->num = 0;
  array->data = NULL;
}

static void
int_array_alloc_contents(int_array *array, int num)
{
  array->num = num;
  array->data = (int *)calloc(num, sizeof(int));
}

static void
int_array_free_contents(int_array *array)
{
  if (array->data != NULL)
    {
      free(array->data);
      array->data = NULL;
    }
}

static void
int_array_add(int_array *array, int value)
{
  int_array old_array = *array;
  array->num++;
  array->data = (int *)calloc(array->num, sizeof(int));
  if (old_array.data != NULL)
    {
      memcpy(array->data, old_array.data,
	     old_array.num * sizeof(int));
      free(old_array.data);
    }
  array->data[old_array.num] = value;
}

static int
int_array_contains(int_array *array, int value)
{
  int i;
  for (i = 0; i < array->num; i++)
    {
      if (array->data[i] == value)
	{
	  return TRUE;
	}
    }
  return FALSE;
}

static void
int_array_copy(int_array *src_array, int_array *dest_array)
{
  dest_array->num = src_array->num;
  if (src_array->data == NULL)
    {
      dest_array->data = NULL;
    } else {
    dest_array->data = (int *)calloc(src_array->num, sizeof(int));
    memcpy(dest_array->data, src_array->data,
	   src_array->num * sizeof(int));
  }
}

static int
compare_ints(const void *a, const void *b)
{
  const int *sa = (const int *) a;
  const int *sb = (const int *) b;
  return *sa > *sb ? 1
    : *sa < *sb ? -1
    : 0;
}

static void
int_array_sort(int_array *array)
{
  qsort(array->data, array->num, sizeof(int), compare_ints);
}

static int
int_array_equals(int_array *array, int_array *other)
{
  return array->num == other->num &&
    memcmp(array->data, other->data, array->num * sizeof(int)) == 0;
}

/*
 * ****************************
 *  int_array_array functions.
 * ****************************
 */

static void
int_array_array_init(int_array_array *array_array)
{
  array_array->num = 0;
  array_array->data = NULL;
}

static void
int_array_array_add_owned(int_array_array *array_array, int_array *array)
{
  int_array_array old_array_array = *array_array;
  array_array->num++;
  array_array->data =
    (int_array *)calloc(array_array->num, sizeof(int_array));
  if (old_array_array.data != NULL)
    {
      memcpy(array_array->data, old_array_array.data,
	     old_array_array.num * sizeof(int_array));
      free(old_array_array.data);
    }
  /* Ownership transfers from right hand side to left hand side. */
  array_array->data[old_array_array.num] = *array;
}

static void
int_array_array_add_copy(int_array_array *array_array, int_array *array)
{
  int_array copy_array;
  int_array_copy(array, &copy_array);
  int_array_array_add_owned(array_array, &copy_array);
}

static int
int_array_array_contains(int_array_array *array_array, int_array *array)
{
  int i;
  for (i = 0; i < array_array->num; i++)
    {
      if (int_array_equals(&array_array->data[i], array))
	{
	  return TRUE;
	}
    }
  return FALSE;
}


static void
int_array_array_alloc_contents(int_array_array *array_array, int num)
{
  array_array->num = num;
  array_array->data = (int_array *)calloc(num, sizeof(int_array));
}

static void
int_array_array_free_contents(int_array_array *array_array)
{
  int i;

  if (array_array->data != NULL)
    {
      for (i = 0; i < array_array->num; i++)
	{
	  int_array_free_contents(&array_array->data[i]);
	}
      free(array_array->data);
      array_array->data = NULL;
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
  cand->letter_index = 0;
  int_array_init(&cand->adapted_stroke_indexes);
}

static void
candidate_copy_deep(candidate *src_cand, candidate *dest_cand)
{
  dest_cand->letter_index = src_cand->letter_index;
  int_array_copy(&src_cand->adapted_stroke_indexes,
		 &dest_cand->adapted_stroke_indexes);
}

static void
candidate_free_contents(candidate *cand)
{
  int_array_free_contents(&cand->adapted_stroke_indexes);
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
candidates_add_owned(candidates *cands, candidate *cand)
{
  candidates old_cands = *cands;
  cands->candidate_num++;
  cands->candidates =
    (candidate *)calloc(cands->candidate_num, sizeof(candidate));
  if (old_cands.candidates != NULL)
    {
      memcpy(cands->candidates, old_cands.candidates,
	     old_cands.candidate_num * sizeof(candidate));
      free(old_cands.candidates);
    }
  /* Ownership transfers from right hand side to left hand side. */
  cands->candidates[old_cands.candidate_num] = *cand;
}

static void
candidates_add_deep_copy(candidates *cands, candidate *cand)
{
  candidate copy_cand;
  candidate_copy_deep(cand, &copy_cand);
  candidates_add_owned(cands, &copy_cand);
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

/*
 * ***********************
 *  data load functions.
 * ***********************
 */

static void
load_data ()
{
  char *p = NULL;
  int letter_num = 0;
  int stroke_num = 0;
  int point_num = 0;
  int i = 0;
  int j = 0;
  int k = 0;
  letter *lttr = NULL;
  stroke *strk = NULL;
  point *pnt = NULL;

  dictionary_alloc_contents(&dict, DICT_LETTER_INITIAL_SIZE);

  FILE *fp = fopen(TOMOEDATADIR "/all.tdic", "r");
  while ((p = fgets(line_buf, LINE_BUF_SIZE, fp)) != NULL)
    {
      if (p[0] == '\n')
	{
	  continue;
	}
      ++letter_num;
      if (letter_num > dict.letter_num)
	{
	  fprintf(stderr, "Abort. This case is not yet implemented.\n");
	  exit(1);
	}

      i = letter_num - 1;
      lttr = &dict.letters[i];
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
	  strk = &lttr->strokes[j];
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
	  stroke_calculate_metrics(strk);
	}
    }
  fclose(fp);

  if (letter_num < dict.letter_num)
    {
      dict.letter_num = letter_num;
    }
}

static int
get_stroke ()
{
  char *p = NULL;
  int stroke_num = 0;
  int point_num = 0;
  int j = 0;
  int k = 0;
  stroke *strk = NULL;
  point *pnt = NULL;
  FILE *fp = stdin;

  /* printf("------------------ get_stroke() ----------------------\n");    */
  while ((p = fgets(line_buf, LINE_BUF_SIZE, fp)) != NULL)
    {
      if (strstr(p, "EOF") != NULL)
	{
	  return FALSE;
	}

      if (p[0] != ':')
	{
	  continue;
	}
      sscanf(p + 1, "%d", &stroke_num);
      /* printf("input_stroke_num=%d\n", stroke_num); */
      letter_alloc_contents(&input, stroke_num);

      for (j = 0; j < stroke_num; j++)
	{
	  strk = &input.strokes[j];
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
	  stroke_calculate_metrics(strk);
	}
      return TRUE;
    }
  return FALSE;
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
  i_met = input_stroke->metrics;
  d_nop = dict_stroke->point_num;
  d_pts = dict_stroke->points;
  d_met = dict_stroke->metrics;

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
		      d_me->a * i_pt->y - d_me->b * i_pt->x - d_me->c <
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
  d_met = dict_stroke->metrics;
  i_nop = input_stroke->point_num;
  i_pts = input_stroke->points;
  i_met = input_stroke->metrics;

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
		      i_me->a * d_pt->y - i_me->b * d_pt->x - i_me->c <
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


static candidates
get_candidates(stroke *input_stroke, candidates cands)
{
  candidates rtn_cands;
  int_array adapted;
  int_array_array cache;
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
  int_array_init(&adapted);
  int_array_array_alloc_contents(&cache, dict.letter_num);

  i_nop = input_stroke->point_num;
  i_pts = input_stroke->points;
  i_met = input_stroke->metrics;

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
  candidates cands = match_char();
  int_array *adapted = NULL;

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

static void
print_matched_characters(int_array *matched)
{
  int i;
  int letter_index;
  for (i = 0; i < matched->num; i++)
    {
      letter_index = matched->data[i];
      if (i > 0)
	{
	  printf(" ");
	}
      printf(dict.letters[letter_index].character);
    }
  printf("\n");
}

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

int
main (int argc, char *argv[])
{
  int_array matched;
  struct timeval tv1;
  struct timezone tz1;
  struct timeval tv2;
  struct timezone tz2;
  struct timeval diff;

  if (argc == 2 && strcmp(argv[1], "-v") == 0)
    {
      verbose = TRUE;
    }

  load_data();

  while (get_stroke())
    {
      if (verbose)
	{
	  gettimeofday(&tv1, &tz1);
	}
      matched = get_matched_char_index();
      if (verbose)
	{
	  gettimeofday(&tv2, &tz2);
	}
      print_matched_characters(&matched);
      if (verbose) {
	timeval_minus(&tv2, &tv1, &diff);
	printf("elapsed time(seconds)=%d.%06d\n",
	       (int)diff.tv_sec, (int)diff.tv_usec);
      }

      letter_free_contents(&input);
    }

  dictionary_free_contents(&dict);
  return 0;
}

#ifdef DEBUG
/*
 * ******************
 *  debug functions.
 * ******************
 */

static char *DEBUG_LOG = "debug.log";

static void
int_array_fprint(FILE *fp, int_array *array)
{
  int i;

  fprintf(fp, "[");
  for (i = 0; i < array->num; i++)
    {
      if (i > 0)
	{
	  fprintf(fp, ", ");
	}
      fprintf(fp, "%d", array->data[i]);
    }
  fprintf(fp, "]");
}

static void
int_array_array_fprint(FILE *fp, int_array_array *array_array)
{
  int i;

  fprintf(fp, "[\n");
  for (i = 0; i < array_array->num; i++)
    {
      int_array_fprint(fp, &array_array->data[i]);
      if (i < array_array->num - 1)
	{
	  fprintf(fp, ",");
	}
      fprintf(fp, "\n");
    }
  fprintf(fp, "]");
}

static void
candidate_fprint(FILE *fp, candidate *cand)
{
  fprintf(fp, "[letter_index=%d,\n", cand->letter_index);
  fprintf(fp, "[adapted_stroke_indexes=");
  int_array_fprint(fp, &cand->adapted_stroke_indexes);
  fprintf(fp, "]");
}

static void
candidates_fprint(FILE *fp, candidates *cands)
{
  int i;
  fprintf(fp, "[candidate_num=%d,\n", cands->candidate_num);
  fprintf(fp, "[candidates=");

  if (cands->candidates != NULL)
    {
      for (i = 0; i < cands->candidate_num; i++)
	{
	  candidate_fprint(fp, &cands->candidates[i]);
	  if (i < cands->candidate_num - 1)
	    {
	      fprintf(fp, ",");
	    }
	  fprintf(fp, "\n");
	}
    }

  fprintf(fp, "]");
}


static FILE *
open_debug_file()
{
  FILE *fp = fopen(DEBUG_LOG, "a");
  return fp;
}

static void
close_debug_file(FILE *fp)
{
  fclose(fp);
}

static char *
int_array_print(int_array *array)
{
  FILE *fp = open_debug_file();
  if (fp == NULL)
    {
      return "failed";
    }
  int_array_fprint(fp, array);
  fprintf(fp, "\n");
  close_debug_file(fp);
  return "ok";
}

static char *
int_array_array_print(int_array_array *array_array)
{
  FILE *fp = open_debug_file();
  if (fp == NULL)
    {
      return "failed";
    }
  int_array_array_fprint(fp, array_array);
  fprintf(fp, "\n");
  close_debug_file(fp);
  return "ok";
}

static char *
candidate_print(candidate *cand)
{
  FILE *fp = open_debug_file();
  if (fp == NULL)
    {
      return "failed";
    }
  candidate_fprint(fp, cand);
  fprintf(fp, "\n");
  close_debug_file(fp);
  return "ok";
}

static char *
candidates_print(candidates *cands)
{
  FILE *fp = open_debug_file();
  if (fp == NULL)
    {
      return "failed";
    }
  candidates_fprint(fp, cands);
  fprintf(fp, "\n");
  close_debug_file(fp);
  return "ok";
}

#endif /* DEBUG */
