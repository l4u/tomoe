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
 *  @file tomoe-config.h
 *  @brief Manages tomoe configuration.
 */

#ifndef __TOMOE_CONFIG_H__
#define __TOMOE_CONFIG_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "tomoe-array.h"

typedef struct _TomoeConfig TomoeConfig;

typedef struct _tomoe_dict_cfg
{
    char*       filename;
    int         dontLoad;
    int         writeAccess;
    int         user;
} tomoe_dict_cfg;

TomoeConfig*    tomoe_config_new                 (const char* config_file);
TomoeConfig*    tomoe_config_add_ref             (TomoeConfig* t_config);
void            tomoe_config_free                (TomoeConfig* t_config);
void            tomoe_config_load                (TomoeConfig* t_config);
void            tomoe_config_save                (TomoeConfig* t_config);
const char*     tomoe_config_get_filename        (TomoeConfig* t_config);
TomoeArray*     tomoe_config_get_dict_list       (TomoeConfig* t_config);
int             tomoe_config_get_default_user_db (TomoeConfig *t_config);

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_CONFIG_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
