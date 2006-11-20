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

/** @file tomoe-handwrite.h
 *  @brief 
 */

#ifndef __TOMOE_HANDWRITE_H__
#define __TOMOE_HANDWRITE_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _TomoePoint     TomoePoint;
typedef struct _TomoeStroke    TomoeStroke;
typedef struct _TomoeGlyph     TomoeGlyph;

struct _TomoePoint
{
    int           x;
    int           y;
};

struct _TomoeStroke
{
    unsigned int  point_num;
    TomoePoint   *points;
};

struct _TomoeGlyph
{
    unsigned int  stroke_num;
    TomoeStroke  *strokes;
};

TomoeStroke    *tomoe_stroke_new                (void);
void            tomoe_stroke_init               (TomoeStroke   *strk,
                                                 int            point_num);
void            tomoe_stroke_clear              (TomoeStroke   *strk);
void            tomoe_stroke_free               (TomoeStroke   *strk);

TomoeGlyph     *tomoe_glyph_new                 (void);
void            tomoe_glyph_init                (TomoeGlyph    *glyph,
                                                 int            stroke_num);
void            tomoe_glyph_clear               (TomoeGlyph    *glyph);
void            tomoe_glyph_free                (TomoeGlyph    *glyph);

G_END_DECLS

#endif /* __TOMOE_HANDWRITE_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
