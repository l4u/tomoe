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
 *  @file tomoe-module.h
 *  @brief Provide a set of API to load shared library dynamically.
 */

#ifndef __TOMOE_MODULE_H__
#define __TOMOE_MODULE_H__

#include <glib-object.h>
#include <gmodule.h>

G_BEGIN_DECLS

typedef gpointer (*TomoeModuleInstantiateFunc) (const gchar *mod_path);

void      tomoe_module_show_error   (GModule     *module);
GModule  *tomoe_module_open         (const gchar *mod_path);
void      tomoe_module_close        (GModule     *module);
gboolean  tomoe_module_load_func    (GModule     *module,
                                     const gchar *func_name,
                                     gpointer    *symbol);
GList    *tomoe_module_load_modules (const gchar *base_dir,
                                     TomoeModuleInstantiateFunc instantiate);
gboolean  tomoe_module_match_name   (GModule     *module,
                                     const gchar *name);

G_END_DECLS

#endif /* __TOMOE_MODULE_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
