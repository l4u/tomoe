/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
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

typedef struct _tomoe_letter    tomoe_letter;
typedef struct _tomoe_glyph     tomoe_glyph;
typedef struct _tomoe_stroke    tomoe_stroke;
typedef struct _tomoe_point     tomoe_point;
typedef struct _tomoe_candidate tomoe_candidate;

struct _tomoe_letter
{
    char         *character;
    tomoe_glyph  *c_glyph;
};

struct _tomoe_glyph
{
    int           stroke_num;
    tomoe_stroke *strokes;
};

struct _tomoe_stroke
{
    int           point_num;
    tomoe_point  *points;
};

struct _tomoe_point
{
    int           x;
    int           y;
};

struct _tomoe_candidate
{
    const char   *letter;
    int           score;
};

/**
 * @brief Free an allocated tomoe_glyph struct.
 * @param glyph - tomoe_glyph structure to free.
 */
void        tomoe_glyph_free    (tomoe_glyph *glyph);

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_LETTER_H__ */
