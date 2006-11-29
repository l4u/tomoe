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
#include "tomoe-module.h"

static GList *dicts = NULL;

void
tomoe_dict_load (const gchar *base_dir)
{
    if (!base_dir)
        base_dir = DICTDIR;

    dicts = g_list_concat (tomoe_module_load_modules (base_dir), dicts);
}

void
tomoe_dict_unload (void)
{
    g_list_foreach (dicts, (GFunc) g_object_unref, NULL);
    g_list_free (dicts);
    dicts = NULL;
}

G_DEFINE_ABSTRACT_TYPE (TomoeDict, tomoe_dict, G_TYPE_OBJECT)

static void
tomoe_dict_class_init (TomoeDictClass *klass)
{
    klass->get_name        = NULL;
    klass->register_char   = NULL;
    klass->unregister_char = NULL;
    klass->get_char        = NULL;
    klass->search          = NULL;
}

static void
tomoe_dict_init (TomoeDict *dict)
{
}

TomoeDict *
tomoe_dict_new (const gchar *name, const gchar *first_property, ...)
{
    GObject *dict;
    va_list var_args;

    va_start (var_args, first_property);
    dict = tomoe_module_instantiate (dicts, name, first_property, var_args);
    va_end (var_args);

    return TOMOE_DICT (dict);
}

const gchar *
tomoe_dict_get_name (TomoeDict *dict)
{
    TomoeDictClass *klass;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), NULL);

    klass = TOMOE_DICT_GET_CLASS (dict);
    if (klass->get_name)
        return klass->get_name (dict);
    else
        return NULL;
}

gboolean
tomoe_dict_register_char (TomoeDict *dict, TomoeChar *chr)
{
    TomoeDictClass *klass;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), FALSE);

    klass = TOMOE_DICT_GET_CLASS (dict);
    if (klass->register_char)
        return klass->register_char (dict, chr);
    else
        return FALSE;
}

gboolean
tomoe_dict_unregister_char (TomoeDict *dict, const gchar *utf8)
{
    TomoeDictClass *klass;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), FALSE);

    klass = TOMOE_DICT_GET_CLASS (dict);
    if (klass->unregister_char)
        return klass->unregister_char (dict, utf8);
    else
        return FALSE;
}

TomoeChar *
tomoe_dict_get_char (TomoeDict *dict, const gchar *utf8)
{
    TomoeDictClass *klass;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), NULL);

    klass = TOMOE_DICT_GET_CLASS (dict);
    if (klass->get_char)
        return klass->get_char (dict, utf8);
    else
        return FALSE;
}

GList *
tomoe_dict_search (TomoeDict *dict, TomoeQuery *query)
{
    TomoeDictClass *klass;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), NULL);

    klass = TOMOE_DICT_GET_CLASS (dict);
    if (klass->search)
        return klass->search (dict, query);
    else
        return NULL;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
