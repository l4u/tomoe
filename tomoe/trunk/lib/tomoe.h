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

/** @file tomoe.h
 *  @brief Provide a set of API for handwriting recognition.
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
#define TRUE  1
#endif

typedef struct _tomoe_glyph     tomoe_glyph;
typedef struct _tomoe_stroke    tomoe_stroke;
typedef struct _tomoe_point     tomoe_point;
typedef struct _tomoe_candidate tomoe_candidate;

/**
 * @typedef typedef int bool;
 *
 * Boolean type used in tomoe library.
 */
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


/**
 * @brief Initialize tomoe
 */
void       tomoe_init          (void);
/**
 * @brief Finalize tomoe library.
 */
void       tomoe_term          (void);

/**
 * @brief Get matched characters 
 * @param input    - matched candidates
 * @return matched - the number of matched characters
 */
int        tomoe_get_matched   (tomoe_glyph       *input,
                                tomoe_candidate ***matched);
/**
 * @brief Free matched characters.
 * @param matched - Matched candidates to free.
 * @param len     - Length of candidates array.
 */
void       tomoe_free_matched  (tomoe_candidate  **matched,
                                int                len);

/**
 * @brief Register to the current user dictionary.
 * @param input - Stroke data.
 * @param data  - Characters to register to the current user dictionary.
 * @param return
 */
tomoe_bool tomoe_data_register (tomoe_glyph       *input,
                                char              *data);

/**
 * @brief Free an allocated tomoe_glyph structure.
 * @param glyph - tomoe_glyph structure to free.
 */
void       tomoe_glyph_free    (tomoe_glyph       *glyph);



#if 0
/* get dictionaries list */
extern int tomoe_get_dictionaries ();

/* set dictionaries */
extern tomoe_bool tomoe_set_dictionaries ();

extern void *tomoe_add_stroke   (void*, tomoe_stroke *stroke);
extern void  tomoe_clear_stroke (void *stroke_list);
#endif



#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_H__ */
