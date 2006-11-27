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

#include <glib-object.h>

G_BEGIN_DECLS

#include "tomoe-char.h"

#define TOMOE_TYPE_DICT            (tomoe_dict_get_type ())
#define TOMOE_DICT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_DICT, TomoeDict))
#define TOMOE_DICT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_DICT, TomoeDictClass))
#define TOMOE_IS_DICT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_DICT))
#define TOMOE_IS_DICT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_DICT))
#define TOMOE_DICT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_DICT, TomoeDictClass))

typedef struct _TomoeDict TomoeDict;
typedef struct _TomoeDictClass TomoeDictClass;

struct _TomoeDict
{
    GObject object;
};

struct _TomoeDictClass
{
    GObjectClass parent_class;
};

GType           tomoe_dict_get_type (void) G_GNUC_CONST;

/**
 * @brief Create a dictionary from a file.
 * @param filename - Name of dictionary file to load.
 * @param editable - Editability of the dictionary.
 * @return Pointer to newly allocated TomoeDict object.
 */
TomoeDict      *tomoe_dict_new                  (const char    *filename,
                                                 gboolean       editable);

/**
 * @brief Get the file name of the tomoe dictionary.
 * @param dict - Pointer to the TomoeDict object.
 * @return File name of the tomoe dictionary. Return NULL if the dictionary
 *         wasn't load from a file, or newly allocated in a program and it
 *         doesn't saved yet.
 */
const char     *tomoe_dict_get_filename         (TomoeDict     *dict);

/**
 * @brief Get the dictionary name.
 * @param dict - Pointer to the TomoeDict object.
 * @return Name of the dictionary.
 */
const char     *tomoe_dict_get_name             (TomoeDict     *dict);

/**
 * @brief Get editable property.
 * @param dict - Pointer to the TomoeDict object.
 * @return TRUE - is editable, FALSE - is not editable
 */
gboolean        tomoe_dict_is_editable          (TomoeDict     *dict);

gboolean        tomoe_dict_is_modified          (TomoeDict     *dict);
void            tomoe_dict_set_modified         (TomoeDict     *dict,
                                                 gboolean      modified);

/**
 * @brief Get character count
 * @param dict - Pointer to the TomoeDict object.
 * @return Count of the characters.
 */
guint           tomoe_dict_get_size             (TomoeDict     *dict);

/* Editable methods */

/**
 * @brief Register a character
 * @param dict - Pointer to the TomoeDict object.
 * @param chr  - Pointer to the TomoeChar object.
 */
gboolean        tomoe_dict_register_char        (TomoeDict     *dict,
                                                 TomoeChar     *chr);

/**
 * @brief Unregister a character by code point
 * @param dict   - Pointer to the TomoeDict object.
 * @param utf8   - UTF-8 encoded value of the character.
 */
gboolean        tomoe_dict_unregister_char      (TomoeDict     *dict,
                                                 const gchar   *utf8);

/**
 * @brief Get a character by code point
 * @param dict  - Pointer to the TomoeDict object.
 * @param utf8  - UTF-8 encoded value of the character.
 * @return TomoeChar object.
 */
TomoeChar      *tomoe_dict_get_char             (TomoeDict     *dict,
                                                 const gchar   *utf8);

/**
 * @brief Return an GPtrArray of TomoeChar which is kept in a TomoeDict.
 * @param dict - Pointer to the TomoeDict struct.
 * @return The array of TomoeChar.
 */
GList          *tomoe_dict_search_by_n_strokes  (TomoeDict     *dict,
                                                 gint           min,
                                                 gint           max);

/* search methods */
/**
 * @brief Match reading of TomoeChar with input.
 * @param dict   - Pointer to the TomoeDict object.
 * @param reading - Pointer to string matchkey
 * @return The GList of TomoeCandidate.
 */
GList           *tomoe_dict_search_by_reading    (TomoeDict     *dict,
                                                  TomoeReading  *reading);

G_END_DECLS

#endif /* __TOMOE_DICT_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
