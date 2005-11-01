/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef struct _tomoe_glyph     tomoe_glyph;
typedef struct _tomoe_stroke    tomoe_stroke;
typedef struct _tomoe_point     tomoe_point;
typedef struct _tomoe_candidate tomoe_candidate;

typedef int tomoe_bool;

struct _tomoe_glyph
{
    int           stroke_num;
    tomoe_stroke *strokes;
};

struct _tomoe_stroke
{
    int          point_num;
    tomoe_point *points;
};

struct _tomoe_point
{
    int x;
    int y;
};

struct _tomoe_candidate
{
    const char  *letter;
    int          score;
};


/* Initialize tomoe */
extern void tomoe_init (void);

#if 0
/* get dictionaries list */
extern int tomoe_get_dictionaries ();

/* set dictionaries */
extern tomoe_bool tomoe_set_dictionaries ();

extern void *tomoe_add_stroke   (void*, tomoe_stroke *stroke);
extern void  tomoe_clear_stroke (void *stroke_list);
#endif

/**
 * @brief get matched characters 
 * @param matched candidates
 * @return the number of matched characters
 *
 */
extern int tomoe_get_matched (tomoe_glyph *input, tomoe_candidate ***matched);

/* 
 * free matched characters 
 *
 * matched:
 *
 */
extern void tomoe_free_matched (tomoe_candidate **matched, int len);

/*
 * register to the current (user?) dictionary
 *
 * input: stroke datas
 * data: characters to register to the current (user?) dictionary
 */
extern tomoe_bool tomoe_data_register (tomoe_glyph *input, char *data);

/* finalize tomoe */
extern void tomoe_term (void);

extern void tomoe_glyph_free (tomoe_glyph *g);

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_H__ */
