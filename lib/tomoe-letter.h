/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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

/** @file tomoe-letter.h
 *  @brief 
 */

#ifndef __TOMOE_LETTER_H__
#define __TOMOE_LETTER_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "tomoe-array.h"

typedef struct _tomoe_point     tomoe_point;
typedef struct _tomoe_stroke    tomoe_stroke;
typedef struct _tomoe_glyph     tomoe_glyph;
typedef struct _tomoe_letter    tomoe_letter;
typedef struct _tomoe_candidate tomoe_candidate;

struct _tomoe_point
{
    int           x;
    int           y;
};

struct _tomoe_stroke
{
    unsigned int  point_num;
    tomoe_point  *points;
};

struct _tomoe_glyph
{
    unsigned int  stroke_num;
    tomoe_stroke *strokes;
};

struct _tomoe_letter
{
    int           ref;
    char*         character;
    tomoe_glyph*  c_glyph;
    tomoe_array*  readings;
    char*         meta;
};

struct _tomoe_candidate
{
    const tomoe_letter* character;
    int           score;
};


tomoe_stroke   *tomoe_stroke_new                (void);
void            tomoe_stroke_init               (tomoe_stroke *strk,
                                                 int           point_num);
void            tomoe_stroke_clear              (tomoe_stroke *strk);
void            tomoe_stroke_free               (tomoe_stroke *strk);
#if 0
void            tomoe_stoke_ref                 (tomoe_stroke *strk);
void            tomoe_stoke_unref               (tomoe_stroke *strk);
unsigned int    tomoe_stroe_get_number_of_points(tomoe_stroke *strk);
void            tomoe_stroke_push_point         (tomoe_stroke *strk,
                                                 ?);
void            tomoe_stroke_pop_point          (tomoe_stroke *strk);
?               tomoe_stroke_get_point          (tomoe_stroke *strk,
                                                 unsigned int  idx);
void            tomoe_stroke_set_point          (tomoe_stroke *strk,
                                                 unsigned int  idx,
                                                 ?);
#endif

tomoe_glyph    *tomoe_glyph_new                 (void);
void            tomoe_glyph_init                (tomoe_glyph *glyph,
                                                 int          stroke_num);
void            tomoe_glyph_clear               (tomoe_glyph *glyph);
/**
 * @brief Free an allocated tomoe_glyph struct.
 * @param glyph - pointer to tomoe_glyph struct to free. NULL pointer will be
 *                ignored.
 */
void            tomoe_glyph_free                (tomoe_glyph *glyph);
#if 0
void            tomoe_glyph_ref                 (tomoe_glyph *glyph);
void            tomoe_glyph_unref               (tomoe_glyph *glyph);
unsigned int    tomoe_glyph_get_number_of_strokes
                                                (tomoe_glyph *glyph);
#endif

/**
 * @brief Create a tomoe letter.
 * @return Pointer to newly allocated tomoe_letter struct.
 */
tomoe_letter*   tomoe_letter_new                (void);

/**
 * @brief Increase reference count.
 * @param this - Pointer to the tomoe_letter struct to increase reference count.
 * @return The tomoe_letter.
 */
tomoe_letter*   tomoe_letter_addref             (tomoe_letter*        this);

/**
 * @brief Decrease reference count and free if zero.
 * @param this - Pointer to the tomoe_letter struct to free.
 */
void            tomoe_letter_free               (tomoe_letter*        this);

/**
 * @brief Compare two tomoe_letter.
 * @param a - Pointer to the tomoe_letter 1 pointer.
 * @param b - Pointer to the tomoe_letter 2 pointer.
 * @return -1 a < b, 0 a= b, 1 a > b
 */
int             tomoe_letter_compare            (const tomoe_letter** a,
                                                 const tomoe_letter** b);

/**
 * @brief Compare two tomoe_candidate.
 * @param a - Pointer to the tomoe_candidate 1 pointer.
 * @param b - Pointer to the tomoe_candidate 2 pointer.
 * @return -1 a < b, 0 a= b, 1 a > b
 */
int             tomoe_candidate_compare         (const tomoe_candidate** a,
                                                 const tomoe_candidate** b);

/**
 * @brief Compare two ansi strings.
 * @param a - Pointer to string 1.
 * @param b - Pointer to string 2.
 * @return -1 a < b, 0 a= b, 1 a > b
 */
int             tomoe_string_compare            (const char**  a,
                                                 const char**  b);

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_LETTER_H__ */
