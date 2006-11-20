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
#include <glib/garray.h>
#include "tomoe.h"
#include "tomoe-dict.h"
#include "tomoe-context.h"
#include "tomoe-config.h"

/* 
 * Initialize tomoe 
 */
void
tomoe_init (void)
{
    static gboolean initialized = FALSE;

    if (!initialized) {
        GTypeDebugFlags debug_flag = G_TYPE_DEBUG_NONE;

        LIBXML_TEST_VERSION;
        g_type_init_with_debug_flags (debug_flag);
    }
}

TomoeContext*
tomoe_simple_load (const char* configFile)
{
    TomoeContext* ctx = tomoe_context_new();
    const GPtrArray* list;
    TomoeConfig* cfg;

    if (!ctx) return NULL;

    cfg = tomoe_config_new (configFile);
    tomoe_config_load (cfg);
    list = tomoe_config_get_dict_list (cfg);
    tomoe_context_load_dict_list (ctx, list);
    g_object_unref (cfg);

    return ctx;
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
