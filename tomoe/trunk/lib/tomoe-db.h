/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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
 *  @file tomoe-db.h
 *  @brief Provides access to the tomoe database.
 */

#ifndef __TOMOE_DB_H__
#define __TOMOE_DB_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "tomoe-data-types.h"
#include "tomoe-dict.h"
#include "tomoe-config.h"

/**
 * @typedef typedef struct _TomoeDB TomoeDB;
 *
 * A struct type which represents tomoe database. All members in it should be
 * accessed through tomoe_dict_* functions.
 */
typedef struct _TomoeDB TomoeDB;

/**
 * @brief Create a database.
 * @return Pointer to newly allocated TomoeDB struct.
 */
TomoeDB        *tomoe_db_new                    (void);

/**
 * @brief Increase reference count.
 * @param t_db     - Pointer to the TomoeDB struct to increase reference count.
 * @return The TomoeDB.
 */
TomoeDB        *tomoe_db_add_ref                (TomoeDB       *t_db);

/**
 * @brief Decrease reference count and free if zero.
 * @param t_db     - Pointer to the TomoeDB struct to free.
 */
void            tomoe_db_free                   (TomoeDB       *t_db);

/**
 * @brief Load dictionary into database.
 * @param t_db     - Pointer to the TomoeDB struct to increase reference count.
 * @param filename - Name of dictionary file to load.
 * @return The TomoeDB.
 */
void            tomoe_db_add_dict               (TomoeDB       *t_db,
                                                 TomoeDict     *dict);
void            tomoe_db_load_dict              (TomoeDB       *t_db,
                                                 const char    *filename,
                                                 int            editable);
void            tomoe_db_load_dict_list         (TomoeDB       *t_db,
                                                 TomoeArray    *list);
TomoeArray     *tomoe_db_get_dict_list          (TomoeDB       *t_db);
void            tomoe_db_save                   (TomoeDB       *db);

#if 0
TomoeDict      *tomoe_db_get_user_dict          (TomoeDB       *t_db,
                                                 const char    *filename);
#endif

/**
 * @brief Match strokes of tomoe_letter with input.
 * @param t_db     - Pointer to the TomoeDB object.
 * @param input    - Pointer to tomoe_glyph matchkey.
 * @return The array of tomoe_candidate.
 */
TomoeArray     *tomoe_db_search_by_strokes      (TomoeDB       *t_db,
                                                 TomoeGlyph    *input);
    
/**
 * @brief Match reading of tomoe_letter with input.
 * @param t_db     - Pointer to the TomoeDB object.
 * @param reading  - Pointer to string matchkey
 * @return The array of tomoe_candidate.
 */
TomoeArray     *tomoe_db_search_by_reading      (TomoeDB       *t_db,
                                                 const char    *reading);
#if 0
/* optional */
void            tomoe_db_enable_dict            (TomoeDB       *t_db,
                                                 const char    *filename);
void            tomoe_db_disable_dict           (TomoeDB       *t_db,
                                                 const char    *filename);
int             tomoe_db_is_dict_enabled        (TomoeDB       *t_db,
                                                 const char    *filename);
TomoeArray*     tomoe_db_get_dicts              (TomoeDB       *t_db);
#endif 

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_DB_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
