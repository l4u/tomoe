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

#ifndef __TOMOE_H__
#define __TOMOE_H__

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef struct _glyph glyph;
typedef struct _stroke stroke;
typedef struct _point point;
typedef struct _candidate candidate;
typedef struct _candidates candidates;

typedef int tomoe_bool;

struct _glyph
{
  int stroke_num;
  stroke *strokes;
};

struct _stroke
{
  int point_num;
  point *points;
};

struct _point
{
  int x;
  int y;
};

struct _candidate
{
  char *letter;
  int score;
};

struct _candidates
{
  int candidate_num;
  candidate *cand;
};

/* Initialize tomoe */
extern void tomoe_init (void);

#if 0
/* get dictionaries list */
extern int tomoe_get_dictionaries ();

/* set dictionaries */
extern tomoe_bool tomoe_set_dictionaries ();

extern void *tomoe_add_stroke (void*, stroke *stroke);
extern void tomoe_clear_stroke (void *stroke_list);
#endif

/* 
 * get matched characters 
 *
 * matched:
 * Return value: the number of matched characters
 *
 */
extern int tomoe_get_matched (glyph *input, candidates **matched);

/* 
 * free matched characters 
 *
 * matched:
 *
 */
extern void tomoe_free_matched (candidates *matched);

/*
 * register to the current (user?) dictionary
 *
 * input: stroke datas
 * data: characters to register to the current (user?) dictionary
 */
extern tomoe_bool tomoe_data_register (glyph *input, char *data);

/* finalize tomoe */
extern void tomoe_term (void);

#endif /* __TOMOE_H__ */
