/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2000 - 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
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

/**
 *  @file tomoe-dict.h
 *  @brief Provide a set of API to access to a tomoe dictionary.
 */

#ifndef __TOMOE_DICT_H__
#define __TOMOE_DICT_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "tomoe-data-types.h"
#include "tomoe-char.h"
#include "tomoe-array.h"

typedef struct _TomoeDict TomoeDict;

/**
 * @brief Create a dictionary from a file.
 * @param filename - Name of dictionary file to load.
 * @param editable - Editability of the dictionary.
 * @return Pointer to newly allocated TomoeDict object.
 */
TomoeDict      *tomoe_dict_new                  (const char    *filename,
                                                 TomoeBool      editable);

/**
 * @brief Increase reference count.
 * @param t_dict - Pointer to the TomoeDict object to increase reference count.
 * @return The TomoeDict.
 */
TomoeDict      *tomoe_dict_add_ref              (TomoeDict     *t_dict);

/**
 * @brief Decrease reference count and free if zero.
 * @param t_dict - Pointer to the TomoeDict object to free.
 */
void            tomoe_dict_free                 (TomoeDict     *t_dict);

/**
 * @brief Save tomoe dictionary.
 * @param t_dict - Pointer to the TomoeDict object.
 */
void            tomoe_dict_save                 (TomoeDict     *t_dict);

/**
 * @brief Get the file name of the tomoe dictionary.
 * @param t_dict - Pointer to the TomoeDict object.
 * @return File name of the tomoe dictionary. Return NULL if the dictionary
 *         wasn't load from a file, or newly allocated in a program and it
 *         doesn't saved yet.
 */
const char     *tomoe_dict_get_filename         (TomoeDict     *t_dict);

/**
 * @brief Get the dictionary name.
 * @param t_dict - Pointer to the TomoeDict object.
 * @return Name of the dictionary.
 */
const char     *tomoe_dict_get_name             (TomoeDict     *t_dict);

/**
 * @brief Get editable property.
 * @param t_dict - Pointer to the TomoeDict object.
 * @return 1 - is editable, 0 - is not editable
 */
TomoeBool       tomoe_dict_is_editable          (TomoeDict     *t_dict);

TomoeBool       tomoe_dict_is_modified          (TomoeDict     *dict);
void            tomoe_dict_set_modified         (TomoeDict     *dict,
                                                 TomoeBool      modified);

/**
 * @brief Get character count
 * @param t_dict - Pointer to the TomoeDict object.
 * @return Count of the characters.
 */
int             tomoe_dict_get_size             (TomoeDict     *t_dict);

/* Editable methods */

/**
 * @brief Add a character
 * @param t_dict - Pointer to the TomoeDict object.
 * @param add  - Pointer to the tomoe_char object.
 */
void            tomoe_dict_add_char             (TomoeDict     *t_dict,
                                                 TomoeChar     *remove);

void            tomoe_dict_insert               (TomoeDict     *dict,
                                                 int            position,
                                                 TomoeChar     *insert);

/**
 * @brief Remove a character by a character object
 * @param t_dict   - Pointer to the TomoeDict object.
 * @param remove - Pointer to the tomoe_char object.
 */
void            tomoe_dict_remove_by_char       (TomoeDict     *t_dict,
                                                 TomoeChar     *remove);

/**
 * @brief Remove a character by index
 * @param t_dict   - Pointer to the TomoeDict object.
 * @param remove - Index of the character.
 */
void            tomoe_dict_remove_by_index      (TomoeDict     *t_dict,
                                                 int            remove);

/**
 * @brief Find a character
 * @param t_dict - Pointer to the TomoeDict object.
 * @param find - Pointer to the tomoe_char object.
 * @return Index of the character.
 */
int             tomoe_dict_find_index           (TomoeDict     *t_dict,
                                                 TomoeChar     *find);

/**
 * @brief Get a character by index
 * @param t_dict  - Pointer to the TomoeDict object.
 * @param index - Index of the character.
 * @return tomoe_char object.
 */
TomoeChar      *tomoe_dict_char_by_index        (TomoeDict     *t_dict,
                                                 int            index);

/**
 * @brief Return an array of tomoe_char which is kept in a TomoeDict.
 * @param t_dict - Pointer to the TomoeDict struct.
 * @return The array of tomoe_char.
 */
TomoeArray     *tomoe_dict_get_letters          (TomoeDict     *t_dict);

/* search methods */

/**
 * @brief Match strokes of tomoe_char with input.
 * @param t_dict   - Pointer to the TomoeDict object.
 * @param input  - Pointer to tomoe_glyph matchkey.
 * @return The array of tomoe_candidate.
 */
TomoeArray     *tomoe_dict_search_by_strokes    (const TomoeDict *t_dict,
                                                 TomoeGlyph      *input);

/**
 * @brief Match reading of tomoe_char with input.
 * @param t_dict   - Pointer to the TomoeDict object.
 * @param input  - Pointer to string matchkey
 * @return The array of tomoe_candidate.
 */
TomoeArray     *tomoe_dict_search_by_reading    (const TomoeDict *t_dict,
                                                 const char      *input);

#ifdef TOMOE_DICT__USE_XSL_METHODS
xsltStylesheetPtr  tomoe_dict_get_meta_xsl      (TomoeDict       *t_dict);
#endif

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_DICT_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
