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

#include "tomoe-dict.h"
#include "tomoe-dict-impl.h"

G_DEFINE_TYPE (TomoeDict, tomoe_dict, TOMOE_TYPE_MODULE)

static void
tomoe_dict_class_init (TomoeDictClass *klass)
{
}

static void
tomoe_dict_init (TomoeDict *dict)
{
}

static void
tomoe_dict_load (TomoeDict *dict, const gchar *filename, gboolean editable)
{
    TomoeModule *module;
    TomoeDictLoadFunc load_func;
    TomoeDictLoadFunc *load_func_p;
    gpointer *p;

    g_return_if_fail (TOMOE_IS_DICT (dict));
    module = TOMOE_MODULE (dict);
    load_func_p = &load_func;
    p = (gpointer *)load_func_p;
    if (tomoe_module_load_func (module,
                                G_STRINGIFY(TOMOE_DICT_IMPL_LOAD),
                                p)) {
        load_func (tomoe_module_get_context (module), filename, editable);
    } else {
        tomoe_module_show_error (module);
    }
}

TomoeDict *
tomoe_dict_new (const char* filename, gboolean editable,
                const gchar *base_dir, const gchar *name)
{
    TomoeDict *dict;

    if (!filename && !*filename) return NULL;

    dict = g_object_new (TOMOE_TYPE_DICT,
                         "default_base_dir", DICTDIR,
                         "new_func_name",
                         G_STRINGIFY(TOMOE_DICT_IMPL_NEW),
                         "free_func_name",
                         G_STRINGIFY(TOMOE_DICT_IMPL_FREE),
                         NULL);
    if (!tomoe_module_find_module (TOMOE_MODULE (dict), base_dir, name)) {
        g_object_unref (dict);
        g_warning ("can't find module for dictionary");
        return NULL;
    }
    tomoe_dict_load (dict, filename, editable);

    return dict;
}

const gchar *
tomoe_dict_get_name (TomoeDict *dict)
{
    TomoeModule *module;
    const gchar *result = NULL;
    TomoeDictGetNameFunc get_name_func;
    TomoeDictGetNameFunc *get_name_func_p;
    gpointer *p;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), result);
    module = TOMOE_MODULE (dict);
    get_name_func_p = &get_name_func;
    p = (gpointer *)get_name_func_p;
    if (tomoe_module_load_func (module,
                                G_STRINGIFY(TOMOE_DICT_IMPL_GET_NAME),
                                p)) {
        result = get_name_func (tomoe_module_get_context (module));
    } else {
        tomoe_module_show_error (module);
    }

    return result;
}

gboolean
tomoe_dict_register_char (TomoeDict *dict, TomoeChar *chr)
{
    TomoeModule *module;
    gboolean result = FALSE;
    TomoeDictRegisterCharFunc register_char_func;
    TomoeDictRegisterCharFunc *register_char_func_p;
    gpointer *p;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), result);
    module = TOMOE_MODULE (dict);
    register_char_func_p = &register_char_func;
    p = (gpointer *)register_char_func_p;
    if (tomoe_module_load_func (module,
                                G_STRINGIFY(TOMOE_DICT_IMPL_REGISTER_CHAR),
                                p)) {
        result = register_char_func (tomoe_module_get_context (module), chr);
    } else {
        tomoe_module_show_error (module);
    }

    return result;
}

gboolean
tomoe_dict_unregister_char (TomoeDict *dict, const gchar *utf8)
{
    TomoeModule *module;
    gboolean result = FALSE;
    TomoeDictUnregisterCharFunc unregister_char_func;
    TomoeDictUnregisterCharFunc *unregister_char_func_p;
    gpointer *p;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), result);
    module = TOMOE_MODULE (dict);
    unregister_char_func_p = &unregister_char_func;
    p = (gpointer *)unregister_char_func_p;
    if (tomoe_module_load_func (module,
                                G_STRINGIFY(TOMOE_DICT_IMPL_UNREGISTER_CHAR),
                                p)) {
        result = unregister_char_func (tomoe_module_get_context (module), utf8);
    } else {
        tomoe_module_show_error (module);
    }

    return result;
}

TomoeChar *
tomoe_dict_get_char (TomoeDict *dict, const gchar *utf8)
{
    TomoeModule *module;
    TomoeChar *result = NULL;
    TomoeDictGetCharFunc get_char_func;
    TomoeDictGetCharFunc *get_char_func_p;
    gpointer *p;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), NULL);
    module = TOMOE_MODULE (dict);
    get_char_func_p = &get_char_func;
    p = (gpointer *)get_char_func_p;
    if (tomoe_module_load_func (module,
                                G_STRINGIFY(TOMOE_DICT_IMPL_GET_CHAR),
                                p)) {
        result = get_char_func (tomoe_module_get_context (module), utf8);
    } else {
        tomoe_module_show_error (module);
    }

    return result;
}

GList *
tomoe_dict_search (TomoeDict *dict, TomoeQuery *query)
{
    TomoeModule *module;
    GList *result = NULL;
    TomoeDictSearchFunc search_func;
    TomoeDictSearchFunc *search_func_p;
    gpointer *p;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), NULL);
    module = TOMOE_MODULE (dict);
    search_func_p = &search_func;
    p = (gpointer *)search_func_p;
    if (tomoe_module_load_func (module,
                                G_STRINGIFY(TOMOE_DICT_IMPL_SEARCH),
                                p)) {
        result = search_func (tomoe_module_get_context (module), query);
    } else {
        tomoe_module_show_error (module);
    }

    return result;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
