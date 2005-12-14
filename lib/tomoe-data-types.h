/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2000 - 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
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

/** @file tomoe-data-types.h
 *  @brief Define fundamental data types.
 */

#ifndef __TOMOE_DATA_TYPES_H__
#define __TOMOE_DATA_TYPES_H__

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @typedef typedef int bool;
 *
 * Boolean type used in tomoe library.
 */
typedef int tomoe_bool;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
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
    int x;
    int y;
};

struct _tomoe_candidate
{
    const char   *letter;
    int           score;
};

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_DATA_TYPES_H__ */
