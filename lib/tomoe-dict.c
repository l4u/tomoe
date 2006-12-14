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
        base_dir = g_getenv ("TOMOE_DICT_MODULE_DIR");
    if (!base_dir)
        base_dir = DICT_MODULEDIR;

    dicts = g_list_concat (tomoe_module_load_modules (base_dir), dicts);
}

void
tomoe_dict_unload (void)
{
    g_list_foreach (dicts, (GFunc) tomoe_module_unload, NULL);
    g_list_free (dicts);
    dicts = NULL;
}

GList *
tomoe_dict_get_registered_types (void)
{
    return tomoe_module_collect_registered_types (dicts);
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
    klass->flush           = NULL;
    klass->is_editable     = NULL;
    klass->get_available_private_utf8 = NULL;
}

static void
tomoe_dict_init (TomoeDict *dict)
{
}

/**
 * tomoe_dict_new:
 * @name: The name of dictionary type.
 * @first_property: the name of the first property.
 * @... :  the value of the first property, followed optionally by more name/value pairs, followed by NULL
 *
 * Create a new #TomoeDict.
 *
 * Return value: a new #TomoeDict.
 */
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

/**
 * tomoe_dict_get_name:
 * @dict: a #TomoeDict.
 *
 * Get the dictionary name.
 *
 * Return value: the name of the dictionary.
 */
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

/**
 * tomoe_dict_register_char:
 * @dict: a TomoeDict object.
 * @chr: a #TomoeChar object to register.
 *
 * Register a #TomoeChar object.
 *
 * Return value: TRUE if success.
 */
gboolean
tomoe_dict_register_char (TomoeDict *dict, TomoeChar *chr)
{
    TomoeDictClass *klass;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), FALSE);

    if (!tomoe_dict_is_editable (dict)) {
        g_warning ("the dictionary isn't editable.");
        return FALSE;
    }

    klass = TOMOE_DICT_GET_CLASS (dict);
    if (klass->register_char) {
        gboolean need_to_set_utf8, success;

        need_to_set_utf8 = !tomoe_char_get_utf8 (chr);
        if (need_to_set_utf8) {
            gchar *utf8;
            utf8 = tomoe_dict_get_available_private_utf8 (dict);
            if (!utf8) {
                g_warning ("there is no available PUA(Private Use Area)");
                return FALSE;
            }
            tomoe_char_set_utf8 (chr, utf8);
            g_free (utf8);
        }
        success = klass->register_char (dict, chr);
        if (!success && need_to_set_utf8)
            tomoe_char_set_utf8 (chr, NULL);
        return success;
    } else {
        return FALSE;
    }
}

/**
 * tomoe_dict_unregister_char:
 * @dict: a TomoeDict object.
 * @utf8: UTF-8 encoded value of the character.
 *
 * Unregister a #TomoeChar object which has utf8 code point.
 *
 * Return value: TRUE if success.
 */
gboolean
tomoe_dict_unregister_char (TomoeDict *dict, const gchar *utf8)
{
    TomoeDictClass *klass;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), FALSE);

    if (!tomoe_dict_is_editable (dict)) {
        g_warning ("the dictionary isn't editable.");
        return FALSE;
    }

    klass = TOMOE_DICT_GET_CLASS (dict);
    if (klass->unregister_char)
        return klass->unregister_char (dict, utf8);
    else
        return FALSE;
}

/**
 * tomoe_dict_get_char:
 * @dict: a TomoeDict object.
 * @utf8: UTF-8 encoded value of the character.
 *
 * Get a #TomoeChar object which has utf8 code point
 *
 * Return value: a #TomoeChar object.
 */
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

gboolean
tomoe_dict_flush (TomoeDict *dict)
{
    TomoeDictClass *klass;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), FALSE);

    klass = TOMOE_DICT_GET_CLASS (dict);
    if (klass->flush)
        return klass->flush (dict);
    else
        return FALSE;
}

gboolean
tomoe_dict_is_editable (TomoeDict *dict)
{
    TomoeDictClass *klass;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), FALSE);

    klass = TOMOE_DICT_GET_CLASS (dict);
    if (klass->is_editable)
        return klass->is_editable (dict);
    else
        return FALSE;
}

gchar *
tomoe_dict_get_available_private_utf8 (TomoeDict *dict)
{
    TomoeDictClass *klass;

    g_return_val_if_fail (TOMOE_IS_DICT (dict), NULL);

    klass = TOMOE_DICT_GET_CLASS (dict);
    if (klass->get_available_private_utf8)
        return klass->get_available_private_utf8 (dict);
    else
        return NULL;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
