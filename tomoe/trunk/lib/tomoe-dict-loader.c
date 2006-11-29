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

#include "tomoe-dict-loader.h"
#include "tomoe-dict-impl.h"

typedef struct _TomoeDictLoader      TomoeDictLoader;
typedef struct _TomoeDictLoaderClass TomoeDictLoaderClass;

static GType tomoe_dict_loader_get_type (void);

#define TOMOE_TYPE_DICT_LOADER    (tomoe_dict_loader_get_type ())
#define TOMOE_DICT_LOADER(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_DICT_LOADER, TomoeDictLoader))
#define TOMOE_IS_DICT_LOADER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_DICT_LOADER))

struct _TomoeDictLoader
{
    GTypeModule parent_instance;

    GModule *module;

    TomoeDictInitFunc init;
    TomoeDictExitFunc exit;
    TomoeDictInstantiateFunc instantiate;

    gchar *mod_path;
};

struct _TomoeDictLoaderClass
{
    GTypeModuleClass parent_class;
};

static GList *dicts = NULL;
static GObjectClass *parent_class = NULL;

static gboolean
load (GTypeModule *module)
{
    TomoeDictLoader *loader = TOMOE_DICT_LOADER (module);

    loader->module = tomoe_module_open (loader->mod_path);
    if (!loader->module)
        return FALSE;

    if (!tomoe_module_load_func (loader->module,
                                 G_STRINGIFY (TOMOE_DICT_IMPL_INIT),
                                 (gpointer )&loader->init) ||
        !tomoe_module_load_func (loader->module,
                                 G_STRINGIFY (TOMOE_DICT_IMPL_EXIT),
                                 (gpointer )&loader->exit) ||
        !tomoe_module_load_func (loader->module,
                                 G_STRINGIFY (TOMOE_DICT_IMPL_INSTANTIATE),
                                 (gpointer )&loader->instantiate))
    {
        tomoe_module_close (loader->module);
        loader->module = NULL;
        return FALSE;
    }

    loader->init (module);

    return TRUE;
}

static void
unload (GTypeModule *module)
{
    TomoeDictLoader *loader = TOMOE_DICT_LOADER (module);

    loader->exit ();

    tomoe_module_close (loader->module);
    loader->module  = NULL;

    loader->init = NULL;
    loader->exit = NULL;
    loader->instantiate = NULL;
}

static void
finalize (GObject *object)
{
    TomoeDictLoader *loader = TOMOE_DICT_LOADER (object);

    g_free (loader->mod_path);
    loader->mod_path = NULL;

    parent_class->finalize (object);
}

G_DEFINE_TYPE (TomoeDictLoader, tomoe_dict_loader, G_TYPE_TYPE_MODULE)

static void
tomoe_dict_loader_class_init (TomoeDictLoaderClass *class)
{
    GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS (class);
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);

    parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (class));

    module_class->load = load;
    module_class->unload = unload;

    gobject_class->finalize = finalize;
}

static void
tomoe_dict_loader_init (TomoeDictLoader *object)
{
}

static gpointer
instantiate (const gchar *mod_path)
{
    TomoeDictLoader *loader;

    loader = g_object_new (TOMOE_TYPE_DICT_LOADER, NULL);
    loader->mod_path = g_strdup (mod_path);
    g_type_module_set_name (G_TYPE_MODULE (loader), loader->mod_path);

    return loader;
}

void
tomoe_dict_loader_load (const gchar *base_dir)
{
    if (!base_dir)
        base_dir = DICTDIR;

    dicts = g_list_concat (tomoe_module_load_modules (base_dir, instantiate),
                           dicts);
}

void
tomoe_dict_loader_unload (void)
{
    g_list_foreach (dicts, (GFunc) g_object_unref, NULL);
    g_list_free (dicts);
    dicts = NULL;
}

TomoeDict *
tomoe_dict_loader_instantiate (const gchar *name, const gchar *filename,
                               gboolean editable)
{
    GList *node;

    for (node = dicts; node; node = g_list_next (node)) {
        TomoeDictLoader *loader = node->data;

        if (g_type_module_use (G_TYPE_MODULE (loader))) {
            TomoeDict *dict = NULL;
            if (tomoe_module_match_name (loader->module, name)) {
                dict = loader->instantiate (filename, editable);
            }
            g_type_module_unuse (G_TYPE_MODULE (loader));
            if (dict)
                return dict;
        }
    }

    return NULL;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
