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

#include "tomoe-data-types.h"
#include "tomoe-object.h"
#include <glib/garray.h>

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

#if 0
TOMOE_CLASS_BEGIN (TomoeCandidate, TomoeObject)
    /* public class members */
    tomoe_char *character;
    int         score;
    /* TOMOE_CLASS_VIRTUAL_SECTION */
    /* section for virtual methods 
    typedef (void) (*myvirt) (void);
    ....
    */
TOMOE_CLASS_END
#endif

/**
 * @brief Compare two TomoeCandidate.
 * @param a - Pointer to the tomoe_candidate 1 pointer.
 * @param b - Pointer to the tomoe_candidate 2 pointer.
 * @return -1 a < b, 0 a= b, 1 a > b
 */
TomoeCandidate *tomoe_candidate_new             (void);
TomoeCandidate *tomoe_candidate_add_ref         (TomoeCandidate  *t_cand);
void            tomoe_candidate_free            (TomoeCandidate  *t_cand);
int             tomoe_candidate_compare         (const TomoeCandidate *a,
                                                 const TomoeCandidate *b);

#ifdef TOMOE_DICT__USE_XSL_METHODS
typedef xsltStylesheetPtr  (*tomoe_dict_interface_get_meta_xsl) (void*);
#else
typedef void*              (*tomoe_dict_interface_get_meta_xsl) (void*);
#endif
typedef TomoeBool          (*tomoe_dict_interface_is_editable)  (void*);
typedef void               (*tomoe_dict_interface_set_modified) (void*,
                                                                 TomoeBool);

typedef struct _tomoe_dict_interface
{
    void*                                instance;
    tomoe_dict_interface_get_meta_xsl    get_meta_xsl;
    tomoe_dict_interface_is_editable     is_editable;
    tomoe_dict_interface_set_modified    set_modified;
} tomoe_dict_interface;

TomoeStroke    *tomoe_stroke_new                (void);
void            tomoe_stroke_init               (TomoeStroke   *strk,
                                                 int            point_num);
void            tomoe_stroke_clear              (TomoeStroke   *strk);
void            tomoe_stroke_free               (TomoeStroke   *strk);
#if 0
void            tomoe_stoke_ref                 (TomoeStroke   *strk);
void            tomoe_stoke_unref               (TomoeStroke   *strk);
unsigned int    tomoe_stroke_get_number_of_points
                                                (TomoeStroke   *strk);
void            tomoe_stroke_push_point         (TomoeStroke   *strk,
                                                 ?);
void            tomoe_stroke_pop_point          (TomoeStroke   *strk);
?               tomoe_stroke_get_point          (TomoeStroke   *strk,
                                                 unsigned int   idx);
void            tomoe_stroke_set_point          (TomoeStroke   *strk,
                                                 unsigned int   idx,
                                                 ?);
#endif

TomoeGlyph     *tomoe_glyph_new                 (void);
void            tomoe_glyph_init                (TomoeGlyph    *glyph,
                                                 int            stroke_num);
void            tomoe_glyph_clear               (TomoeGlyph    *glyph);
/**
 * @brief Free an allocated TomoeGlyph struct.
 * @param glyph - pointer to TomoeGlyph struct to free. NULL pointer will be
 *                ignored.
 */
void            tomoe_glyph_free                (TomoeGlyph    *glyph);
#if 0
void            tomoe_glyph_ref                 (TomoeGlyph    *glyph);
void            tomoe_glyph_unref               (TomoeGlyph    *glyph);
unsigned int    tomoe_glyph_get_number_of_strokes
                                                (TomoeGlyph    *glyph);
#endif

/**
 * @brief Create a tomoe letter.
 * @return Pointer to newly allocated tomoe_letter struct.
 */
TomoeChar      *tomoe_char_new                  (tomoe_dict_interface *dict);

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
void             tomoe_char_free                 (TomoeChar*     t_char);

const char      *tomoe_char_get_code             (const TomoeChar *t_char);
void             tomoe_char_set_code             (TomoeChar*     t_char,
                                                  const char*    code);
GPtrArray       *tomoe_char_get_readings         (TomoeChar*     t_char);
void             tomoe_char_set_readings         (TomoeChar*     t_char,
                                                  GPtrArray*     readings);
TomoeGlyph      *tomoe_char_get_glyph            (TomoeChar*     t_char);
void             tomoe_char_set_glyph            (TomoeChar*     t_char,
                                                  TomoeGlyph*    glyph);
const char      *tomoe_char_get_meta             (TomoeChar*     t_char);
void             tomoe_char_set_dict_interface   (TomoeChar*     chr,
                                                  tomoe_dict_interface *parent);
TomoeBool        tomoe_char_is_editable          (TomoeChar*     chr);
TomoeBool        tomoe_char_is_modified          (TomoeChar*     chr);
void             tomoe_char_set_modified         (TomoeChar*     chr,
                                                 TomoeBool      modified);

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
