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

#include <glib-object.h>

G_BEGIN_DECLS

#define TOMOE_TYPE_CONFIG            (tomoe_config_get_type ())
#define TOMOE_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_CONFIG, TomoeConfig))
#define TOMOE_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_CONFIG, TomoeConfigClass))
#define TOMOE_IS_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_CONFIG))
#define TOMOE_IS_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_CONFIG))
#define TOMOE_CONFIG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_CONFIG, TomoeConfigClass))

typedef struct _TomoeConfig TomoeConfig;
typedef struct _TomoeConfigClass TomoeConfigClass;

struct _TomoeConfig
{
    GObject object;
};

struct _TomoeConfigClass
{
    GObjectClass parent_class;
};

typedef struct _TomoeDictCfg
{
    gchar       *filename;
    gint         dontLoad;
    gint         writeAccess;
    gint         user;
} TomoeDictCfg;

GType            tomoe_config_get_type (void) G_GNUC_CONST;

TomoeConfig     *tomoe_config_new                 (const char   *config_file);
void             tomoe_config_load                (TomoeConfig  *config);
void             tomoe_config_save                (TomoeConfig  *config);
const gchar     *tomoe_config_get_filename        (TomoeConfig  *config);
const GPtrArray *tomoe_config_get_dict_list       (TomoeConfig  *config);
gint             tomoe_config_get_default_user_db (TomoeConfig  *config);

G_END_DECLS

#endif /* __TOMOE_CONFIG_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
