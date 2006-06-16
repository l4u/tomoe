/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2000 - 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2006 Juernjakob Harder <juernjakob.harder@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your opion) any later version.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tomoe.h"
#include "tomoe-dict.h"
#include "tomoe-db.h"
#include "tomoe-array.h"

/* 
 * Initialize tomoe 
 */
tomoe_db*
tomoe_init (void)
{
    tomoe_db* db = tomoe_db_new();
    if (!db) return NULL;
    /* load all available dictionaries */
    tomoe_db_add_dict(db, TOMOEDATADIR "/all.xml");
    tomoe_db_add_dict(db, TOMOEDATADIR "/readingtest.xml");
    tomoe_db_add_dict(db, TOMOEDATADIR "/kanjidic2.xml");
    return db;
}

/* finalize tomoe */
void 
tomoe_term (void)
{
}


/* register characters with stroke */
tomoe_bool
tomoe_data_register (tomoe_glyph *input, char *data)
{
    tomoe_bool ret = FALSE;

    /* Not implemented yet*/
    return ret;
}



/*
vi:ts=4:nowrap:ai:expandtab
*/
