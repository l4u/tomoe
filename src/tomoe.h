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

typedef struct _letter letter;
typedef struct _stroke stroke;
typedef struct _point point;
typedef struct _metric metric;

struct _letter
{
  char *character;
  int stroke_num;
  stroke *strokes;
};

struct _stroke
{
  int point_num;
  point *points;
  metric *metrics;
};

struct _point
{
  int x;
  int y;
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

/* Initialize tomoe */
void tomoe_init (void);

/* 
 * get matched characters 
 *
 * matched:
 * Return value: the number of matched characters
 *
 */
int tomoe_match (letter *input, char ***matched);

/* 
 * free matched characters 
 *
 * matched:
 *
 */
void tomoe_free_matched (char **matched);

/* finalize tomoe */
void tomoe_term (void);

#endif /* __TOMOE_H__ */
