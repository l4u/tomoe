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

#include <gmodule.h>
#include <glib/gi18n.h>
#include "tomoe-module.h"

#define TOMOE_MODULE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_MODULE, TomoeModulePrivate))

typedef struct _TomoeModulePrivate TomoeModulePrivate;
struct _TomoeModulePrivate
{
    GModule *module;
    void *context;

    gchar *default_base_dir;
    gchar *new_func_name;
    gchar *free_func_name;
};

enum
{
    PROP_0,
    PROP_NAME,
    PROP_BASE_DIR,
    PROP_DEFAULT_BASE_DIR,
    PROP_NEW_FUNC_NAME,
    PROP_FREE_FUNC_NAME
};

G_DEFINE_TYPE (TomoeModule, tomoe_module, G_TYPE_OBJECT)

static void dispose          (GObject *object);
static void set_property     (GObject         *object,
                              guint            prop_id,
                              const GValue    *value,
                              GParamSpec      *pspec);

static void
tomoe_module_class_init (TomoeModuleClass *klass)
{
    GObjectClass *gobject_class;
    GParamSpec *spec;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = dispose;
    gobject_class->set_property = set_property;

    spec = g_param_spec_string ("default_base_dir",
                                N_("Default base directory"),
                                N_("Default base directory for loading or "
                                   "searching module."),
                                NULL,
                                G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_property (gobject_class, PROP_DEFAULT_BASE_DIR,
                                     spec);

    spec = g_param_spec_string ("new_func_name",
                                N_("New function name"),
                                N_("New function name of the module."),
                                NULL,
                                G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_property (gobject_class, PROP_NEW_FUNC_NAME, spec);

    spec = g_param_spec_string ("free_func_name",
                                N_("Free function name"),
                                N_("Free function name of the module."),
                                NULL,
                                G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_property (gobject_class, PROP_FREE_FUNC_NAME, spec);

    g_type_class_add_private (gobject_class, sizeof (TomoeModulePrivate));
}

static void
tomoe_module_init (TomoeModule *module)
{
    TomoeModulePrivate *priv = TOMOE_MODULE_GET_PRIVATE (module);

    priv->module = NULL;
    priv->context = NULL;
}

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
    TomoeModulePrivate *priv;

    priv = TOMOE_MODULE_GET_PRIVATE (object);
    switch (prop_id) {
      case PROP_DEFAULT_BASE_DIR:
        g_free (priv->default_base_dir);
        priv->default_base_dir = g_value_dup_string (value);
        break;
      case PROP_NEW_FUNC_NAME:
        g_free (priv->new_func_name);
        priv->new_func_name = g_value_dup_string (value);
        break;
      case PROP_FREE_FUNC_NAME:
        g_free (priv->free_func_name);
        priv->free_func_name = g_value_dup_string (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

gpointer
tomoe_module_get_context (TomoeModule *module)
{
    TomoeModulePrivate *priv;

    g_return_val_if_fail (TOMOE_IS_MODULE (module), NULL);
    priv = TOMOE_MODULE_GET_PRIVATE (module);
    return priv->context;
}


void
tomoe_module_show_error (TomoeModule *module)
{
    TomoeModulePrivate *priv;
    gchar *message;

    g_return_if_fail (TOMOE_IS_MODULE (module));
    priv = TOMOE_MODULE_GET_PRIVATE (module);

    if (!g_module_error ()) return;
    message = g_locale_to_utf8 (g_module_error(), -1, NULL, NULL, NULL);

    if (priv->module) {
        gchar *name;
        name = g_strdup (g_module_name (priv->module));
        g_warning ("%s: %s", name, message);
        g_free (name);
    } else {
        g_warning ("%s", message);
    }

    g_free (message);
}

gboolean
tomoe_module_load_func (TomoeModule *module, const gchar *func_name,
                        gpointer *symbol)
{
    TomoeModulePrivate *priv;

    g_return_val_if_fail (TOMOE_IS_MODULE (module), FALSE);
    priv = TOMOE_MODULE_GET_PRIVATE (module);

    g_return_val_if_fail (priv->module, FALSE);
    if (g_module_symbol (priv->module, func_name, symbol)) {
        return TRUE;
    } else {
        tomoe_module_show_error (module);
        return FALSE;
    }
}

static void
tomoe_module_close_module (TomoeModule *module)
{
    TomoeModulePrivate *priv;
    TomoeModuleFreeFunc free_func;
    TomoeModuleFreeFunc *free_func_p;
    gpointer *p;

    g_return_if_fail (TOMOE_IS_MODULE (module));
    priv = TOMOE_MODULE_GET_PRIVATE (module);

    g_return_if_fail (priv->module);

    free_func_p = &free_func;
    p = (gpointer *)free_func_p;
    if (tomoe_module_load_func (module, priv->free_func_name, p))
        free_func (priv->context);
    else
        tomoe_module_show_error (module);

    if (!g_module_close (priv->module))
        tomoe_module_show_error (module);
    priv->module = NULL;
    priv->context = NULL;
}

static gboolean
tomoe_module_load(TomoeModule *module, const gchar *base_dir, const gchar *name)
{
    TomoeModulePrivate *priv;
    gboolean success = FALSE;
    gchar *mod_path;

    g_return_val_if_fail (TOMOE_IS_MODULE (module), FALSE);
    priv = TOMOE_MODULE_GET_PRIVATE (module);

    mod_path = g_module_build_path (base_dir, name);
    priv->module = g_module_open (mod_path, G_MODULE_BIND_LAZY);
    priv->context = NULL;

    if (priv->module) {
        TomoeModuleNewFunc new_func;
        TomoeModuleNewFunc *new_func_p;
        gpointer *p;

        new_func_p = &new_func;
        p = (gpointer *)new_func_p;
        if (tomoe_module_load_func (module, priv->new_func_name, p)) {
            priv->context = new_func ();
            success = TRUE;
        } else {
            tomoe_module_close_module (module);
        }
    } else {
        tomoe_module_show_error (NULL);
    }

    g_free (mod_path);

    return success;
}

gboolean
tomoe_module_find_module(TomoeModule *module,
                         const gchar *base_dir, const gchar *name)
{
    TomoeModulePrivate *priv;
    GDir *dir;

    g_return_val_if_fail (TOMOE_IS_MODULE (module), FALSE);
    priv = TOMOE_MODULE_GET_PRIVATE (module);

    if (!base_dir)
        base_dir = priv->default_base_dir;

    if (name) {
        tomoe_module_load (module, base_dir, name);
    } else {
        dir = g_dir_open (base_dir, 0, NULL);
        if (dir) {
            const gchar *entry;

            while ((entry = g_dir_read_name(dir))) {
                if (tomoe_module_load (module, base_dir, entry))
                    break;
            }

            g_dir_close(dir);
        }
    }

    return priv->module ? TRUE : FALSE;
}

static void
dispose (GObject *object)
{
    TomoeModule *module;
    module = TOMOE_MODULE (object);
    TomoeModulePrivate *priv;

    priv = TOMOE_MODULE_GET_PRIVATE (module);

    if (priv->module) {
        tomoe_module_close_module (module);
    }

    g_free (priv->default_base_dir);
    g_free (priv->new_func_name);
    g_free (priv->free_func_name);

    priv->default_base_dir = NULL;
    priv->new_func_name = NULL;
    priv->free_func_name = NULL;

    G_OBJECT_CLASS (tomoe_module_parent_class)->dispose (object);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
