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
#include "tomoe-letter.h"
#include "array.h"

/**
 * @typedef typedef struct _tomoe_dict tomoe_dict;
 *
 * A struct type which represents tomoe dictionary. All members in it should be
 * accessed through tomoe_dict_* functions.
 */
typedef struct _tomoe_dict tomoe_dict;

/**
 * @brief Load a dictionary from a file.
 * @param filename - Name of dictionary file to load.
 * @return Pointer to newly allocated tomoe_dict struct.
 */
tomoe_dict  *tomoe_dict_new           (const char    *filename);

/**
 * @brief Free an allocated tomoe_dict struct.
 * @param dict - Pointer to the tomoe_dict struct to free.
 */
void         tomoe_dict_free          (tomoe_dict    *dict);

/**
 * @brief Increase reference count.
 * @param dict - Pointer to the tomoe_dict struct to increase reference count.
 * @return The tomoe_dict.
 */
tomoe_dict  *tomoe_dict_addref        (tomoe_dict    *dict);

/**
 * @brief Get the file name of the tomoe dictionary.
 * @param dict - Pointer to the tomoe_dict struct.
 * @return File name of the tomoe dictionary. Return NULL if the dictionary
 *         wasn't load from a file, or newly allocated in a program and it
 *         doesn't saved yet.
 */
const char  *tomoe_dict_get_file_name (tomoe_dict    *dict);

/**
 * @brief Get the dictionary name.
 * @param dict - Pointer to the tomoe_dict struct.
 * @return Name of the dictionary.
 */
const char  *tomoe_dict_get_name      (tomoe_dict    *dict);

/**
 * @brief Reurn an array of tomoe_letter which is kept in a tomoe_dict.
 * @param dict - Pointer to the tomoe_dict struct.
 * @return The array of tomoe_letter.
 */
tomoe_array* tomoe_dict_get_letters   (tomoe_dict    *dict);

/**
 * @brief
 * @param dict   -
 * @param letter -
 */
tomoe_array* tomoe_dict_get_matched   (tomoe_dict*    this,
                                       tomoe_glyph*   input);


#if 0
/*
 * Not implemented yet.
 */
/**
 * @brief
 * @param dict   -
 * @param letter -
 */
tomoe_array* tomoe_dict_get_reading   (tomoe_dict*    this,
                                       const char*    reading);

/**
 * @brief
 * @param dict   -
 * @param letter -
 */
void         tomoe_dict_append_letter (tomoe_dict    *dict,
                                       tomoe_letter  *letter);

/**
 * @brief
 * @param dict   -
 * @param letter -
 */
void         tomoe_dict_remove_letter (tomoe_dict    *dict,
                                       tomoe_letter  *letter);
#endif

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_DICT_H__ */
