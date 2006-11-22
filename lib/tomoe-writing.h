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

#include <glib-object.h>

G_BEGIN_DECLS

#define TOMOE_TYPE_GLYPH            (tomoe_glyph_get_type ())
#define TOMOE_GLYPH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_GLYPH, TomoeGlyph))
#define TOMOE_GLYPH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_GLYPH, TomoeGlyphClass))
#define TOMOE_IS_GLYPH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_GLYPH))
#define TOMOE_IS_GLYPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_GLYPH))
#define TOMOE_GLYPH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_GLYPH, TomoeGlyphClass))

typedef struct _TomoeGlyph      TomoeGlyph;
typedef struct _TomoeGlyphClass TomoeGlyphClass;

typedef struct _TomoePoint        TomoePoint;

struct _TomoeGlyph
{
    GObject object;
};

struct _TomoeGlyphClass
{
    GObjectClass parent_class;
};

struct _TomoePoint
{
    gint x;
    gint y;
};

GType           tomoe_glyph_get_type            (void) G_GNUC_CONST;
TomoeGlyph     *tomoe_glyph_new                 (void);
void            tomoe_glyph_move_to             (TomoeGlyph    *glyph,
                                                 gint           x,
                                                 gint           y);
void            tomoe_glyph_line_to             (TomoeGlyph    *glyph,
                                                 gint           x,
                                                 gint           y);
void            tomoe_glyph_clear               (TomoeGlyph    *glyph);
guint           tomoe_glyph_get_number_of_strokes
                                                (TomoeGlyph    *glyph);
guint           tomoe_glyph_get_number_of_points(TomoeGlyph    *glyph,
                                                 guint          stroke);
gboolean        tomoe_glyph_get_point           (TomoeGlyph    *glyph,
                                                 guint          stroke,
                                                 guint          point,
                                                 gint          *x,
                                                 gint          *y);
gboolean        tomoe_glyph_get_last_point      (TomoeGlyph    *glyph,
                                                 gint          *x,
                                                 gint          *y);
void            tomoe_glyph_remove_last_stroke  (TomoeGlyph    *glyph);

const GList    *tomoe_glyph_get_strokes         (TomoeGlyph    *glyph);

G_END_DECLS

#endif /* __TOMOE_HANDWRITE_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
