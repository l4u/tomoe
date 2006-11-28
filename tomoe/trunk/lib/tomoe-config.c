/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <glib.h>
#include <glib/gi18n.h>

#include "tomoe-config.h"
#include "glib-utils.h"

#define TOMOE_CONFIG_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_CONFIG, TomoeConfigPrivate))

typedef struct _TomoeConfigPrivate  TomoeConfigPrivate;
struct _TomoeConfigPrivate
{
    gchar       *filename;
    GKeyFile    *key_file;
};

enum
{
    PROP_0,
    PROP_FILENAME
};


G_DEFINE_TYPE (TomoeConfig, tomoe_config, G_TYPE_OBJECT)

static const gchar *system_config_file = TOMOESYSCONFDIR "/config";

static void     tomoe_config_dispose      (GObject       *object);
static void     tomoe_config_set_property (GObject       *object,
                                           guint          prop_id,
                                           const GValue  *value,
                                           GParamSpec    *pspec);
static void     tomoe_config_get_property (GObject       *object,
                                           guint          prop_id,
                                           GValue        *value,
                                           GParamSpec    *pspec);

static void     tomoe_config_save         (TomoeConfig *config);

static gboolean _tomoe_dict_key_file_get_boolean_value (GKeyFile *key_file,
                                                        const gchar *group,
                                                        const gchar *key,
                                                        gboolean default_value);
static void     _tomoe_dict_load_system_dictionaries   (TomoeConfig *config,
                                                        TomoeShelf *shelf);

static void
tomoe_config_class_init (TomoeConfigClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose      = tomoe_config_dispose;
    gobject_class->set_property = tomoe_config_set_property;
    gobject_class->get_property = tomoe_config_get_property;

    g_object_class_install_property (gobject_class,
                                     PROP_FILENAME,
                                     g_param_spec_string ("filename",
                                         N_("Filename"),
                                         N_("The Filename for storing user settings"),
                                         NULL,
                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_type_class_add_private (gobject_class, sizeof (TomoeConfigPrivate));
}

static void
tomoe_config_init (TomoeConfig *config)
{
    TomoeConfigPrivate *priv = TOMOE_CONFIG_GET_PRIVATE (config);

    priv->filename   = NULL;
    priv->key_file   = NULL;
}

TomoeConfig *
tomoe_config_new (const gchar *config_file)
{
    TomoeConfig *config;

    config = g_object_new(TOMOE_TYPE_CONFIG,
                          "filename", config_file,
                          NULL);

    return config;
}

static void
tomoe_config_dispose (GObject *object)
{
    TomoeConfig *config;
    TomoeConfigPrivate *priv;

    config = TOMOE_CONFIG (object);
    priv = TOMOE_CONFIG_GET_PRIVATE (config);

    tomoe_config_save (config);

    if (priv->filename) {
        g_free (priv->filename);
    }
    if (priv->key_file) {
        g_key_file_free (priv->key_file);
    }

    priv->filename  = NULL;
    priv->key_file = NULL;

    if (G_OBJECT_CLASS (tomoe_config_parent_class)->dispose)
        G_OBJECT_CLASS (tomoe_config_parent_class)->dispose (object);
}

static void
tomoe_config_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    TomoeConfigPrivate *priv = TOMOE_CONFIG_GET_PRIVATE (object);

    switch (prop_id) {
      case PROP_FILENAME:
        priv->filename = g_value_dup_string (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
tomoe_config_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    TomoeConfigPrivate *priv = TOMOE_CONFIG_GET_PRIVATE (object);

    switch (prop_id) {
      case PROP_FILENAME:
        g_value_set_string (value, priv->filename);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

void
tomoe_config_load (TomoeConfig *config)
{
    GKeyFile *key_file;
    GError *error = NULL;
    TomoeConfigPrivate *priv;
    const gchar *config_file;

    g_return_if_fail (config);

    priv = TOMOE_CONFIG_GET_PRIVATE (config);

    if (priv->key_file) {
        g_key_file_free (priv->key_file);
        priv->key_file = NULL;
    }

    key_file = g_key_file_new ();
    config_file = priv->filename ? priv->filename : system_config_file;
    if (!g_key_file_load_from_file (key_file, config_file,
                                    G_KEY_FILE_KEEP_COMMENTS |
                                    G_KEY_FILE_KEEP_TRANSLATIONS,
                                    &error)) {
        TOMOE_HANDLE_ERROR (error);
        return;
    }

    priv->key_file = key_file;
}

static void
tomoe_config_save (TomoeConfig *config)
{
    TomoeConfigPrivate *priv;

    g_return_if_fail (config);

    priv = TOMOE_CONFIG_GET_PRIVATE (config);
    if (priv->filename && priv->key_file) {
        gchar *data;
        gsize length;
        GError *error = NULL;

        data = g_key_file_to_data (priv->key_file, &length, &error);
        if (error) {
            TOMOE_HANDLE_ERROR (error);
            return;
        }

        if (!g_file_set_contents (priv->filename, data, length, &error)) {
            TOMOE_HANDLE_ERROR (error);
            return;
        }
    }
}

TomoeShelf *
tomoe_config_make_shelf (TomoeConfig *config)
{
    TomoeConfigPrivate *priv;
    TomoeShelf *shelf;
    GKeyFile *key_file;
    guint i;
    gchar **dicts;
    gsize dicts_size;

    g_return_val_if_fail (config, NULL);

    priv = TOMOE_CONFIG_GET_PRIVATE(config);
    key_file = priv->key_file;
    g_return_val_if_fail (key_file, NULL);

    shelf = tomoe_shelf_new ();
    dicts = g_key_file_get_groups (key_file, &dicts_size);
    for (i = 0; i < dicts_size; i++) {
        GError *error = NULL;
        gchar *filename, *dict_name;
        TomoeDict *dict;

        dict_name = dicts[i];
        if (!g_str_has_suffix (dict_name, "-dictionary"))
            continue;

        filename = g_key_file_get_string (key_file, dict_name, "file", &error);
        if (error) {
            TOMOE_HANDLE_ERROR (error);
            continue;
        }

        if (!_tomoe_dict_key_file_get_boolean_value (key_file, dict_name,
                                                     "use", TRUE))
            continue;

        if (_tomoe_dict_key_file_get_boolean_value (key_file, dict_name,
                                                    "user", TRUE)) {
            dict = tomoe_dict_new (filename, TRUE);
        } else {
            gchar *dict_filename;
            dict_filename = g_build_filename (TOMOEDATADIR, filename, NULL);
            dict = tomoe_dict_new (dict_filename, FALSE);
            g_free (dict_filename);
        }

        if (dict) {
            tomoe_shelf_add_dict (shelf, dict);
            g_object_unref (dict);
        }

        g_free (filename);
    }

    if (_tomoe_dict_key_file_get_boolean_value (key_file,
                                                "config",
                                                "use_system_dictionaries",
                                                TRUE)) {
        _tomoe_dict_load_system_dictionaries (config, shelf);
    }

    g_strfreev(dicts);
    return shelf;
}

static gboolean
_tomoe_dict_key_file_get_boolean_value (GKeyFile *key_file,
                                        const gchar *group,
                                        const gchar *key,
                                        gboolean default_value)
{
    gboolean result;
    GError *error = NULL;

    result = g_key_file_get_boolean (key_file, group, key, &error);
    if (error) {
        switch (error->code) {
          case G_KEY_FILE_ERROR_NOT_FOUND:
            g_error_free (error);
            break;
          case G_KEY_FILE_ERROR_INVALID_VALUE:
            TOMOE_HANDLE_ERROR (error);
            break;
        }
        result = default_value;
    }

    return result;
}

static void
_tomoe_dict_load_system_dictionaries (TomoeConfig *config, TomoeShelf *shelf)
{
    const gchar *filename;
    GDir *gdir;

    gdir = g_dir_open (TOMOEDATADIR, 0, NULL);
    while ((filename = g_dir_read_name (gdir))) {
        TomoeDict *dict;
        gchar *path;

        if (!g_str_has_suffix (filename, ".xml"))
            continue;
        path = g_build_filename(TOMOEDATADIR, filename, NULL);
        if (tomoe_shelf_has_dict (shelf, path)) {
            g_free (path);
            continue;
        }

        dict = tomoe_dict_new (path, FALSE);
        if (dict) {
            tomoe_shelf_add_dict (shelf, dict);
            g_object_unref (dict);
        }

        g_free (path);
    }
    g_dir_close (gdir);
}

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
