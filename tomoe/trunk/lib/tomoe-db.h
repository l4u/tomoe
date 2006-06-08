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

typedef struct _tomoe_db tomoe_db;

tomoe_db*     tomoe_db_new                (void);
tomoe_db*     tomoe_db_addref             (tomoe_db*     this);
void          tomoe_db_free               (tomoe_db*     this);

void          tomoe_db_add_dict           (tomoe_db*     this,
                                           const char*   filename);
tomoe_array*  tomoe_db_get_matched        (tomoe_db*     this,
                                           tomoe_glyph*  input);
tomoe_array*  tomoe_db_get_reading        (tomoe_db*     this,
                                           const char*   reading);
#if 0
/* optional */
void          tomoe_db_enable_dict        (tomoe_db*     this,
                                           const char*   filename);
void          tomoe_db_disable_dict       (tomoe_db*     this,
                                           const char*   filename);
int           tomoe_db_is_dict_enabled    (tomoe_db*     this,
                                           const char*   filename);
tomoe_array*  tomoe_db_get_dicts          (tomoe_db*     this);
#endif 

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_DB_H__ */
