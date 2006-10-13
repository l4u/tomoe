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

typedef struct _tomoe_dict tomoe_dict;

/**
 * @brief Create a dictionary from a file.
 * @param filename - Name of dictionary file to load.
 * @param editable - Editability of the dictionary.
 * @return Pointer to newly allocated tomoe_dict object.
 */
tomoe_dict  *tomoe_dict_new           (const char    *filename,
                                       tomoe_bool     editable);

/**
 * @brief Increase reference count.
 * @param t_dict - Pointer to the tomoe_dict object to increase reference count.
 * @return The tomoe_dict.
 */
tomoe_dict*  tomoe_dict_addref        (tomoe_dict*    t_dict);

/**
 * @brief Decrease reference count and free if zero.
 * @param t_dict - Pointer to the tomoe_dict object to free.
 */
void         tomoe_dict_free          (tomoe_dict*    t_dict);

/**
 * @brief Save tomoe dictionary.
 * @param t_dict - Pointer to the tomoe_dict object.
 */
void         tomoe_dict_save          (tomoe_dict*    t_dict);

/**
 * @brief Get the file name of the tomoe dictionary.
 * @param t_dict - Pointer to the tomoe_dict object.
 * @return File name of the tomoe dictionary. Return NULL if the dictionary
 *         wasn't load from a file, or newly allocated in a program and it
 *         doesn't saved yet.
 */
const char*  tomoe_dict_getFilename   (tomoe_dict    *t_dict);

/**
 * @brief Get the dictionary name.
 * @param t_dict - Pointer to the tomoe_dict object.
 * @return Name of the dictionary.
 */
const char*  tomoe_dict_getName       (tomoe_dict*    t_dict);

/**
 * @brief Get editable property.
 * @param t_dict - Pointer to the tomoe_dict object.
 * @return 1 - is editable, 0 - is not editable
 */
tomoe_bool   tomoe_dict_getEditable   (tomoe_dict*    t_dict);

tomoe_bool   tomoe_dict_get_modified  (tomoe_dict    *dict);
void         tomoe_dict_set_modified  (tomoe_dict    *dict,
                                       tomoe_bool     modified);

/**
 * @brief Get character count
 * @param t_dict - Pointer to the tomoe_dict object.
 * @return Count of the characters.
 */
int          tomoe_dict_getSize       (tomoe_dict*    t_dict);

/* Editable methods */

/**
 * @brief Add a character
 * @param t_dict - Pointer to the tomoe_dict object.
 * @param add  - Pointer to the tomoe_char object.
 */
void         tomoe_dict_addChar       (tomoe_dict*    t_dict,
                                       tomoe_char*    remove);

void         tomoe_dict_insert        (tomoe_dict    *dict,
                                       int            position,
                                       tomoe_char    *insert);

/**
 * @brief Remove a character by a character object
 * @param t_dict   - Pointer to the tomoe_dict object.
 * @param remove - Pointer to the tomoe_char object.
 */
void         tomoe_dict_removeByChar  (tomoe_dict*    t_dict,
                                       tomoe_char*    remove);

/**
 * @brief Remove a character by index
 * @param t_dict   - Pointer to the tomoe_dict object.
 * @param remove - Index of the character.
 */
void         tomoe_dict_removeByIndex (tomoe_dict*    t_dict,
                                       int            remove);

/**
 * @brief Find a character
 * @param t_dict - Pointer to the tomoe_dict object.
 * @param find - Pointer to the tomoe_char object.
 * @return Index of the character.
 */
int          tomoe_dict_findIndex     (tomoe_dict*    t_dict,
                                       tomoe_char*    find);

/**
 * @brief Get a character by index
 * @param t_dict  - Pointer to the tomoe_dict object.
 * @param index - Index of the character.
 * @return tomoe_char object.
 */
tomoe_char*  tomoe_dict_charByIndex   (tomoe_dict*    t_dict,
                                       int index);

/**
 * @brief Return an array of tomoe_char which is kept in a tomoe_dict.
 * @param t_dict - Pointer to the tomoe_dict struct.
 * @return The array of tomoe_char.
 */
tomoe_array* tomoe_dict_get_letters   (tomoe_dict    *t_dict);

/* search methods */

/**
 * @brief Match strokes of tomoe_char with input.
 * @param t_dict   - Pointer to the tomoe_dict object.
 * @param input  - Pointer to tomoe_glyph matchkey.
 * @return The array of tomoe_candidate.
 */
tomoe_array* tomoe_dict_searchByStrokes   (const tomoe_dict* t_dict,
                                                    tomoe_glyph*      input);

/**
 * @brief Match reading of tomoe_char with input.
 * @param t_dict   - Pointer to the tomoe_dict object.
 * @param input  - Pointer to string matchkey
 * @return The array of tomoe_candidate.
 */
tomoe_array* tomoe_dict_searchByReading   (const tomoe_dict* t_dict,
                                                 const char*       input);

#ifdef TOMOE_DICT__USE_XSL_METHODS
xsltStylesheetPtr  tomoe_dict_getMetaXsl        (tomoe_dict*       t_dict);
#endif

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_DICT_H__ */
