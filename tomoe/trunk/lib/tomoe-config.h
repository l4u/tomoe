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

typedef struct _tomoe_config tomoe_config;

typedef struct _tomoe_dict_cfg
{
    char*       filename;
    int         dontLoad;
    int         writeAccess;
    int         user;
} tomoe_dict_cfg;

tomoe_config* tomoe_config_new         (const char* configFile);
tomoe_config* tomoe_config_addref      (tomoe_config* this);
void          tomoe_config_free        (tomoe_config* this);
void          tomoe_config_load        (tomoe_config* this);
void          tomoe_config_save        (tomoe_config* this);
const char*   tomoe_config_getFilename (tomoe_config* this);
tomoe_array*  tomoe_config_getDictList (tomoe_config* this);
int           tomoe_config_getDefaultUserDB (tomoe_config *this);

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_CONFIG_H__ */
 
