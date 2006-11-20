/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <string.h>
#include <sys/stat.h>
#include <libxml/xmlreader.h>
#include <glib.h>
#include <glib/gi18n.h>

#include "tomoe-config.h"
#include "glib-utils.h"

#define TOMOE_CONFIG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_CONFIG, TomoeConfigPrivate))

typedef struct _TomoeConfigPrivate	TomoeConfigPrivate;
struct _TomoeConfigPrivate
{
    gchar       *filename;
    gint         use_system_dictionaries;
    GPtrArray   *dict_list;
    gint         default_user_db;
};

enum
{
	PROP_0,
	PROP_FILENAME
};


G_DEFINE_TYPE (TomoeConfig, tomoe_config, G_TYPE_OBJECT)

static const xmlChar* defaultConfig  = BAD_CAST "<?xml version=\"1.0\" standalone=\"no\"?>" \
                                "	<tomoeConfig>" \
                                "	<useSystemDictionaries value=\"yes\"/>" \
                                "</tomoeConfig>";

static const gchar *system_config_file = TOMOESYSCONFDIR "/config.xml";
static const gchar *default_config_file = "/config.xml";

static void     tomoe_config_finalize     (GObject       *object);
static void     tomoe_config_dispose      (GObject       *object);
static GObject *tomoe_config_constructor  (GType                  type,
                                           guint                  n_props,
                                           GObjectConstructParam *props);
static void     tomoe_config_set_property (GObject       *object,
                                           guint          prop_id,
                                           const GValue  *value,
                                           GParamSpec    *pspec);
static void     tomoe_config_get_property (GObject       *object,
                                           guint          prop_id,
                                           GValue        *value,
                                           GParamSpec    *pspec);

static TomoeDictCfg*   _tomoe_dict_cfg_new      (void);
static void            _tomoe_dict_cfg_free     (gpointer data, gpointer user_data);
static gint            _tomoe_dict_cfg_cmp      (gconstpointer a, gconstpointer b);
static void            _tomoe_create_config_dir (void);

static void
tomoe_config_class_init (TomoeConfigClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize     = tomoe_config_finalize;
    gobject_class->dispose      = tomoe_config_dispose;
    gobject_class->constructor  = tomoe_config_constructor;
	gobject_class->set_property = tomoe_config_set_property;
	gobject_class->get_property = tomoe_config_get_property;

	g_object_class_install_property (gobject_class,
                                     PROP_FILENAME,
                                     g_param_spec_string ("filename",
                                         N_("Filename"),
                                         N_("The Filename for storing user settings"),
                                         NULL,
                                         G_PARAM_READWRITE));

    g_type_class_add_private (gobject_class, sizeof (TomoeConfigPrivate));
}

static GObject *
tomoe_config_constructor (GType type, guint n_props,
                          GObjectConstructParam *props)
{
	GObject *object;
    TomoeConfigPrivate *priv;
	GObjectClass *klass = G_OBJECT_CLASS (tomoe_config_parent_class);

	object = klass->constructor (type, n_props, props);
    priv = TOMOE_CONFIG_GET_PRIVATE (object);

    /* check config file */
    if (!priv->filename || !g_file_test (priv->filename, G_FILE_TEST_EXISTS)) {
        /* use ~/.tomoe/config.xml */
        const gchar *home = g_get_home_dir ();

        if (!home) {
            priv->filename = NULL;
        } else {
            _tomoe_create_config_dir ();
            priv->filename = g_build_filename (home, "."PACKAGE, default_config_file, NULL);
            /* if not found, use systemConfigFile */
            if (!g_file_test (priv->filename, G_FILE_TEST_EXISTS) &&
                g_file_test (system_config_file, G_FILE_TEST_EXISTS))
            {
                FILE *src_file, *dest_file;
                char buf[4096];
                size_t bytes;

                src_file = fopen (system_config_file, "r");
                dest_file = fopen (priv->filename, "w");
                while (!feof(src_file))
                {
                    bytes = fread (buf, 1, sizeof (buf), src_file);
                    fwrite (buf, 1, bytes, dest_file);
                }

                fclose (src_file);
                fclose (dest_file);
            }
        }
    }

	return object;
}

static void
tomoe_config_init (TomoeConfig *config)
{
    TomoeConfigPrivate *priv = TOMOE_CONFIG_GET_PRIVATE (config);

    priv->filename   = NULL;
    priv->dict_list  = g_ptr_array_new ();
}

TomoeConfig *
tomoe_config_new (const char *config_file)
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
    TomoeConfigPrivate *priv = TOMOE_CONFIG_GET_PRIVATE (object);

    if (priv->filename) {
        g_free (priv->filename);
    }

    if (priv->dict_list) {
        TOMOE_PTR_ARRAY_FREE_ALL (priv->dict_list, _tomoe_dict_cfg_free);
    }
    priv->filename  = NULL;
    priv->dict_list = NULL;

	if (G_OBJECT_CLASS (tomoe_config_parent_class)->dispose)
		G_OBJECT_CLASS (tomoe_config_parent_class)->dispose (object);
}

static void
tomoe_config_finalize (GObject *object)
{
	if (G_OBJECT_CLASS (tomoe_config_parent_class)->finalize)
		G_OBJECT_CLASS (tomoe_config_parent_class)->finalize (object);
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
    xmlDocPtr doc;
    xmlNodePtr root;
    char* defaultUserDB = NULL;
    int i;
    TomoeConfigPrivate *priv;

    g_return_if_fail (config);

    priv = TOMOE_CONFIG_GET_PRIVATE (config);
    doc = priv->filename ? xmlReadFile (priv->filename, NULL, 1)
                         : xmlReadDoc (defaultConfig, NULL, NULL, 1);

    root = xmlDocGetRootElement(doc);

    priv->default_user_db = -1;
    priv->use_system_dictionaries = 0;

    if (root && 0 == xmlStrcmp(root->name, BAD_CAST "tomoeConfig")) {
        xmlNodePtr node;

        for (node = root->children; node; node = node->next) {
            if (node->type != XML_ELEMENT_NODE)
                continue;

            if (0 == xmlStrcmp(node->name, BAD_CAST "useSystemDictionaries")) {
                priv->use_system_dictionaries = 1;
            } else if (0 == xmlStrcmp(node->name, BAD_CAST "defaultUserDB")) {
                xmlAttrPtr prop;

                for (prop = node->properties; prop; prop = prop->next) {
                    if (0 == xmlStrcmp(prop->name, BAD_CAST "file"))
                        defaultUserDB = g_strdup ((const gchar*)prop->children->content);
                }
            } else if (0 == xmlStrcmp(node->name, BAD_CAST "dictionary")) {
                xmlAttrPtr prop;
                TomoeDictCfg* dcfg = _tomoe_dict_cfg_new ();

                dcfg->writeAccess = 0;
                dcfg->dontLoad = 0;
                dcfg->user = 1;
                for (prop = node->properties; prop; prop = prop->next) {
                    if (0 == xmlStrcmp(prop->name, BAD_CAST "file"))
                        dcfg->filename = g_strdup ((const gchar*)prop->children->content);
                    else if (0 == xmlStrcmp(prop->name, BAD_CAST "dontLoad"))
                        dcfg->dontLoad = xmlStrcmp(prop->children->content, BAD_CAST "yes") ? 0 : 1;
                    else if (0 == xmlStrcmp(prop->name, BAD_CAST "system"))
                        dcfg->user = xmlStrcmp(prop->children->content, BAD_CAST "yes") ? 1 : 0;
                }

                /* check if file exists */
                fprintf (stdout, dcfg->filename);
                /*if (access (dcfg->filename, F_OK | R_OK)) FIXME*/
                {/*fprintf(stdout, "..access ok\n");*/
                    dcfg->writeAccess = /*access (dcfg->filename, W_OK) ? */dcfg->user/* : 0*/;
                    g_ptr_array_add (priv->dict_list, dcfg);
                }
                /*
                else
                    {_tomoe_dict_cfg_free (dcfg);fprintf(stdout,"..not found\n");}
                */
            }

        }
    }
    xmlFreeDoc (doc);

    g_ptr_array_sort (priv->dict_list, _tomoe_dict_cfg_cmp);

    if (defaultUserDB) {
        for (i = 0; i < priv->dict_list->len; i++) {
            TomoeDictCfg* dcfg = (TomoeDictCfg*)g_ptr_array_index (priv->dict_list, i);

            if (strcmp (defaultUserDB, dcfg->filename) == 0) {
                priv->default_user_db = i;
                break;
            }
        }
    }

    /* search in TOMOEDATADIR for additional dictionaries */
    if (priv->use_system_dictionaries) {
        const gchar *filename;
        GDir *gdir;

        gdir = g_dir_open (TOMOEDATADIR, 0, NULL);
        while ((filename = g_dir_read_name (gdir))) {
            gboolean dup = FALSE;

            if (!g_str_has_suffix (filename, ".xml"))
                continue;
            for (i = 0; i < priv->dict_list->len; i++) {
                TomoeDictCfg *dcfg = g_ptr_array_index (priv->dict_list, i);
                if (!strcmp (dcfg->filename, filename)) {
                    dup = TRUE;
                }
            }

            if (!dup ) {
                TomoeDictCfg* dcfg = _tomoe_dict_cfg_new ();
                dcfg->writeAccess = 0;
                dcfg->dontLoad = 0;
                dcfg->user = 0;
                dcfg->filename = g_strdup (filename);
                g_ptr_array_add (priv->dict_list, dcfg);
            }
        }
        g_dir_close (gdir);

        g_ptr_array_sort (priv->dict_list, _tomoe_dict_cfg_cmp);
    }
}

void
tomoe_config_save (TomoeConfig *config)
{
    TomoeConfigPrivate *priv;

    g_return_if_fail (config);

    priv = TOMOE_CONFIG_GET_PRIVATE (config);
    if (priv->filename) {
        xmlDocPtr doc;
        const char* param[3];
        xmlNodePtr root;
        int i;

        doc = xmlNewDoc(BAD_CAST "1.0");
        root = xmlNewNode(NULL, BAD_CAST "tomoeConfig");
        param[0] = 0;

        xmlDocSetRootElement (doc, root);

        if (priv->use_system_dictionaries)
            xmlNewChild (root, NULL, BAD_CAST "useSystemDictionaries", NULL);


        for (i = 0; i < priv->dict_list->len; i++) {
            xmlNodePtr node = xmlNewChild (root, NULL, BAD_CAST "dictionary", NULL);
            TomoeDictCfg *dict = (TomoeDictCfg*)g_ptr_array_index (priv->dict_list, i);

            xmlNewProp (node, BAD_CAST "file", BAD_CAST dict->filename);
            if (!dict->user)
                xmlNewProp (node, BAD_CAST "system", BAD_CAST "yes");
            if (dict->dontLoad)
                xmlNewProp (node, BAD_CAST "dontLoad", BAD_CAST "yes");
        }

        xmlSaveFormatFileEnc(priv->filename, doc, "UTF-8", 1);
        xmlFreeDoc (doc);
    }
}

const GPtrArray *
tomoe_config_get_dict_list (TomoeConfig *config)
{
    TomoeConfigPrivate *priv;

    g_return_val_if_fail (config, NULL);

    priv = TOMOE_CONFIG_GET_PRIVATE (config);
    return priv->dict_list;
}

gint
tomoe_config_get_default_user_db (TomoeConfig *config)
{
    TomoeConfigPrivate *priv;

    g_return_val_if_fail (config, 0);

    priv = TOMOE_CONFIG_GET_PRIVATE (config);
    return priv->default_user_db;
}

static TomoeDictCfg*
_tomoe_dict_cfg_new (void)
{
    TomoeDictCfg* p = (TomoeDictCfg*)calloc (1, sizeof (TomoeDictCfg));

    p->filename = NULL;
    return p;
}

static void
_tomoe_dict_cfg_free (gpointer data, gpointer user_data)
{
    TomoeDictCfg *p = (TomoeDictCfg*) data;
    if (!data) return;
    g_free (p->filename);
    g_free (p);
}

static gint
_tomoe_dict_cfg_cmp (gconstpointer a, gconstpointer b)
{
    TomoeDictCfg *ca, *cb;
    ca = (TomoeDictCfg*) a;
    cb = (TomoeDictCfg*) b;
    return ca->user == cb->user ? strcmp(ca->filename, cb->filename) 
                                : cb->user - ca->user;
}

static void
_tomoe_create_config_dir (void)
{
    gchar *path;
    const gchar *home;

    home = g_get_home_dir ();
    if (!home)
        return;

    path = g_build_filename (home, "."PACKAGE, NULL);

    if (!g_file_test (path, G_FILE_TEST_IS_DIR)) {
        g_free (path);
        return;
    }

    mkdir (path, 0711);
    g_free (path);
}

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
