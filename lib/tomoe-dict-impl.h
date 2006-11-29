/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Kouhei Sutou <kou@cozmixng.org>
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
 *  @file tomoe-dict-impl.h
 *  @brief Provide a set of API to access to a tomoe dictionary.
 */

#ifndef __TOMOE_DICT_IMPL_H__
#define __TOMOE_DICT_IMPL_H__

#include <glib-object.h>

G_BEGIN_DECLS

#include "tomoe-dict.h"

typedef void             (*TomoeDictInitFunc)        (GTypeModule *module);
typedef void             (*TomoeDictExitFunc)        (void);
typedef TomoeDict       *(*TomoeDictInstantiateFunc) (const gchar *filename,
                                                      gboolean     editable);

#define TOMOE_DICT_IMPL_INIT        tomoe_dict_impl_init
#define TOMOE_DICT_IMPL_EXIT        tomoe_dict_impl_exit
#define TOMOE_DICT_IMPL_INSTANTIATE tomoe_dict_impl_instantiate


void             TOMOE_DICT_IMPL_INIT        (GTypeModule  *module);
void             TOMOE_DICT_IMPL_EXIT        (void);
GObject         *TOMOE_DICT_IMPL_INSTANTIATE (const gchar *first_property, va_list args);

G_END_DECLS

#endif /* __TOMOE_DICT_IMPL_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
