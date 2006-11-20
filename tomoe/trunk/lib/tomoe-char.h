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

#ifndef __TOMOE_CHAR_H__
#define __TOMOE_CHAR_H__

#include <glib-object.h>

G_BEGIN_DECLS

#include <glib/garray.h>

typedef struct _TomoeDict TomoeDict;

typedef struct _TomoePoint     TomoePoint;
typedef struct _TomoeStroke    TomoeStroke;
typedef struct _TomoeGlyph     TomoeGlyph;
typedef struct _TomoeChar      TomoeChar;
typedef struct _TomoeCandidate TomoeCandidate;

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

struct _TomoeCandidate
{
    int           ref;
    TomoeChar    *character;
    int           score;
};

TomoeCandidate *tomoe_candidate_new             (void);
TomoeCandidate *tomoe_candidate_add_ref         (TomoeCandidate  *t_cand);
void            tomoe_candidate_free            (TomoeCandidate  *t_cand);

/**
 * @brief Compare two TomoeCandidate.
 * @param a - Pointer to the tomoe_candidate 1 pointer.
 * @param b - Pointer to the tomoe_candidate 2 pointer.
 * @return -1 a < b, 0 a= b, 1 a > b
 */
int             tomoe_candidate_compare         (const TomoeCandidate *a,
                                                 const TomoeCandidate *b);

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

/**
 * @brief Create a tomoe letter.
 * @return Pointer to newly allocated tomoe_letter struct.
 */
TomoeChar      *tomoe_char_new                  (TomoeDict     *dict);

/**
 * @brief Increase reference count.
 * @param t_char - Pointer to the tomoe_letter struct to increase reference count.
 * @return The tomoe_letter.
 */
TomoeChar      *tomoe_char_add_ref              (TomoeChar*     t_char);

/**
 * @brief Decrease reference count and free if zero.
 * @param t_char - Pointer to the tomoe_letter struct to free.
 */
void            tomoe_char_free                 (TomoeChar*     t_char);

const char     *tomoe_char_get_code             (const TomoeChar *t_char);
void            tomoe_char_set_code             (TomoeChar*     t_char,
                                                 const char*    code);
GPtrArray      *tomoe_char_get_readings         (TomoeChar*     t_char);
void            tomoe_char_set_readings         (TomoeChar*     t_char,
                                                 GPtrArray*     readings);
TomoeGlyph     *tomoe_char_get_glyph            (TomoeChar*     t_char);
void            tomoe_char_set_glyph            (TomoeChar*     t_char,
                                                 TomoeGlyph*    glyph);
const char     *tomoe_char_get_meta             (TomoeChar*     t_char);
void            tomoe_char_set_dict             (TomoeChar*     chr,
                                                 TomoeDict     *parent);
gboolean        tomoe_char_is_editable          (TomoeChar*     chr);
gboolean        tomoe_char_is_modified          (TomoeChar*     chr);
void            tomoe_char_set_modified         (TomoeChar*     chr,
                                                 gboolean      modified);

#ifdef TOMOE_CHAR__USE_XML_METHODS
xmlNodePtr      tomoe_char_get_xml_meta         (TomoeChar*     t_char);
void            tomoe_char_set_xml_meta         (TomoeChar*     t_char,
                                                 xmlNodePtr     meta);
void            tomoe_char_set_meta_xsl         (TomoeChar*     t_char,
                                                 xsltStylesheetPtr  metaXsl);
#endif

/**
 * @brief Compare two tomoe_letter.
 * @param a - Pointer to the tomoe_letter 1 pointer.
 * @param b - Pointer to the tomoe_letter 2 pointer.
 * @return -1 a < b, 0 a= b, 1 a > b
 */
gint            tomoe_char_compare              (const TomoeChar *a,
                                                 const TomoeChar *b);


/**
 * @brief Compare two ansi strings.
 * @param a - Pointer to string 1.
 * @param b - Pointer to string 2.
 * @return -1 a < b, 0 a= b, 1 a > b
 */
int             tomoe_string_compare            (const char **a,
                                                 const char **b);

G_END_DECLS

/*interface_tomoe_array (TomoeCandidateArray, const TomoeCandidate*);*/

#endif /* __TOMOE_CHAR_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
