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

#include <stdlib.h>
#include <string.h>

#include "tomoe-module.h"

void
tomoe_module_show_error (GModule *module)
{
    gchar *message;

    if (!g_module_error ()) return;
    message = g_locale_to_utf8 (g_module_error(), -1, NULL, NULL, NULL);

    if (module) {
        gchar *name;
        name = g_strdup (g_module_name (module));
        g_warning ("%s: %s", name, message);
        g_free (name);
    } else {
        g_warning ("%s", message);
    }

    g_free (message);
}

GModule *
tomoe_module_open (const gchar *mod_path)
{
    GModule *module;

    module = g_module_open (mod_path, G_MODULE_BIND_LAZY);
    if (!module) {
        tomoe_module_show_error (NULL);
    }

    return module;
}

void
tomoe_module_close (GModule *module)
{
    if (module && g_module_close (module)) {
        tomoe_module_show_error (NULL);
    }
}

gboolean
tomoe_module_load_func (GModule *module, const gchar *func_name,
                        gpointer *symbol)
{
    g_return_val_if_fail (module, FALSE);

    if (g_module_symbol (module, func_name, symbol)) {
        return TRUE;
    } else {
        tomoe_module_show_error (module);
        return FALSE;
    }
}

static gpointer
tomoe_module_load (const gchar *base_dir, const gchar *name,
                   TomoeModuleInstantiateFunc instantiate)
{
    gchar *mod_path;
    gpointer result = NULL;

    mod_path = g_module_build_path (base_dir, name);
    if (g_str_has_suffix (mod_path, G_MODULE_SUFFIX)) {
        result = instantiate (mod_path);
    }
    g_free (mod_path);

    return result;
}

GList *
tomoe_module_load_modules (const gchar *base_dir,
                           TomoeModuleInstantiateFunc instantiate)
{
    GDir *dir;
    GList *results = NULL;

    dir = g_dir_open (base_dir, 0, NULL);
    if (dir) {
        const gchar *entry;

        while ((entry = g_dir_read_name(dir))) {
            gpointer result;
            result = tomoe_module_load (base_dir, entry, instantiate);
            if (result)
                results = g_list_prepend (results, result);
        }

        g_dir_close(dir);
    }

    return results;
}

gboolean
tomoe_module_match_name (GModule *module, const gchar *name)
{
    gboolean matched;
    gchar *module_base_name, *normalized_matched_name;

    module_base_name = g_path_get_basename (g_module_name (module));
    normalized_matched_name = g_module_build_path (NULL, name);

    matched = (0 == strcmp (module_base_name, normalized_matched_name));

    g_free (module_base_name);
    g_free (normalized_matched_name);

    return matched;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
