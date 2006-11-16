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
#include <libxml/xmlreader.h>
#include "tomoe.h"
#include "tomoe-dict.h"
#include "tomoe-db.h"
#include "tomoe-array.h"
#include "tomoe-config.h"

/* 
 * Initialize tomoe 
 */
void
tomoe_init (void)
{
    LIBXML_TEST_VERSION
}

TomoeDB*
tomoe_simple_load (const char* configFile)
{
    TomoeDB* db = tomoe_db_new();
    TomoeArray* list;
    TomoeConfig* cfg;

    if (!db) return NULL;

    cfg = tomoe_config_new (configFile);
    tomoe_config_load (cfg);
    list = tomoe_config_get_dict_list (cfg);
    tomoe_db_load_dict_list (db, list);
    tomoe_config_free (cfg);

    return db;
}

/* finalize tomoe */
void 
tomoe_quit (void)
{
    /* TODO unfreed tomoe objects check */
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
