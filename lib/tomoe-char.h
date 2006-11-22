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

/** @file tomoe-char.h
 *  @brief 
 */

#ifndef __TOMOE_CHAR_H__
#define __TOMOE_CHAR_H__

#include <glib-object.h>

G_BEGIN_DECLS

#include <glib/garray.h>
#include <tomoe-writing.h>

#define TOMOE_TYPE_CHAR            (tomoe_char_get_type ())
#define TOMOE_CHAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_CHAR, TomoeChar))
#define TOMOE_CHAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_CHAR, TomoeCharClass))
#define TOMOE_IS_CHAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_CHAR))
#define TOMOE_IS_CHAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_CHAR))
#define TOMOE_CHAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_CHAR, TomoeCharClass))

typedef struct _TomoeChar      TomoeChar;
typedef struct _TomoeCharClass TomoeCharClass;

struct _TomoeChar
{
    GObject object;
};

struct _TomoeCharClass
{
    GObjectClass parent_class;
};

GType           tomoe_char_get_type (void) G_GNUC_CONST;

/**
 * @brief Create a TomoeChar.
 * @return Pointer to newly allocated TomoeChar struct.
 */
TomoeChar      *tomoe_char_new                  (void);

const char     *tomoe_char_get_code             (const TomoeChar *t_char);
void            tomoe_char_set_code             (TomoeChar     *t_char,
                                                 const char    *code);
GPtrArray      *tomoe_char_get_readings         (TomoeChar     *t_char);
void            tomoe_char_set_readings         (TomoeChar     *t_char,
                                                 GPtrArray     *readings);
TomoeGlyph     *tomoe_char_get_glyph            (TomoeChar     *t_char);
void            tomoe_char_set_glyph            (TomoeChar     *t_char,
                                                 TomoeGlyph    *glyph);

/**
 * @brief Compare two tomoe_letter.
 * @param a - Pointer to the tomoe_letter 1 pointer.
 * @param b - Pointer to the tomoe_letter 2 pointer.
 * @return -1 a < b, 0 a= b, 1 a > b
 */
gint            tomoe_char_compare              (const TomoeChar *a,
                                                 const TomoeChar *b);

const char     *tomoe_char_get_meta_data        (TomoeChar     *chr,
                                                 const gchar   *key);
void            tomoe_char_register_meta_data   (TomoeChar     *chr,
                                                 const gchar   *key,
                                                 const gchar   *value);
void            tomoe_char_meta_data_foreach    (TomoeChar* chr,
                                                 GHFunc func,
                                                 gpointer user_data);

G_END_DECLS

#endif /* __TOMOE_CHAR_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
