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

typedef gpointer   (*TomoeDictNewFunc)            (GHashTable  *hash);
typedef gpointer   (*TomoeDictFreeFunc)           (gpointer     context);
typedef gchar     *(*TomoeDictGetNameFunc)        (gpointer     context);
typedef gboolean   (*TomoeDictRegisterCharFunc)   (gpointer     context,
                                                   TomoeChar   *chr);
typedef gboolean   (*TomoeDictUnregisterCharFunc) (gpointer     context,
                                                   const gchar *utf8);
typedef TomoeChar *(*TomoeDictGetCharFunc)        (gpointer     context,
                                                   const gchar *utf8);
typedef GList     *(*TomoeDictSearchFunc)         (gpointer     context,
                                                   TomoeQuery  *query);

typedef void       (*TomoeDictLoadFunc)           (gpointer     context,
                                                   const gchar *filename,
                                                   gboolean     editable);

#define TOMOE_DICT_IMPL_NEW              tomoe_dict_impl_new
#define TOMOE_DICT_IMPL_FREE             tomoe_dict_impl_free
#define TOMOE_DICT_IMPL_GET_NAME         tomoe_dict_impl_get_name
#define TOMOE_DICT_IMPL_REGISTER_CHAR    tomoe_dict_impl_register_char
#define TOMOE_DICT_IMPL_UNREGISTER_CHAR  tomoe_dict_impl_unregister_char
#define TOMOE_DICT_IMPL_GET_CHAR         tomoe_dict_impl_get_char
#define TOMOE_DICT_IMPL_SEARCH           tomoe_dict_impl_search

#define TOMOE_DICT_IMPL_LOAD             tomoe_dict_impl_load

gpointer     TOMOE_DICT_IMPL_NEW               (void);
void         TOMOE_DICT_IMPL_FREE              (gpointer     context);
const gchar *TOMOE_DICT_IMPL_GET_NAME          (gpointer     context);
gboolean     TOMOE_DICT_IMPL_REGISTER_CHAR     (gpointer     context,
                                                TomoeChar   *chr);
gboolean     TOMOE_DICT_IMPL_UNREGISTER_CHAR   (gpointer     context,
                                                const gchar *utf8);
TomoeChar   *TOMOE_DICT_IMPL_GET_CHAR          (gpointer     context,
                                                const gchar *utf8);
GList       *TOMOE_DICT_IMPL_SEARCH            (gpointer     context,
                                                TomoeQuery  *query);

void         TOMOE_DICT_IMPL_LOAD              (gpointer     context,
                                                const gchar *filename,
                                                gboolean     editable);

G_END_DECLS

#endif /* __TOMOE_DICT_IMPL_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
