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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libxml/xmlreader.h>
#include <glib.h>
#include <glob.h>

#include "tomoe-config.h"
#include "glib-utils.h"

static const xmlChar* defaultConfig  = BAD_CAST "<?xml version=\"1.0\" standalone=\"no\"?>" \
                                "	<tomoeConfig>" \
                                "	<useSystemDictionaries value=\"yes\"/>" \
                                "</tomoeConfig>";

static const char* systemConfigFile = TOMOESYSCONFDIR "/config.xml";
static const char* defaultConfigFile = "/config.xml";

static TomoeDictCfg*   _tomoe_dict_cfg_new      (void);
static void            _tomoe_dict_cfg_free     (gpointer data, gpointer user_data);
static gint            _tomoe_dict_cfg_cmp      (gconstpointer a, gconstpointer b);
static void            _tomoe_create_config_dir (void);

struct _TomoeConfig
{
    int          ref;
    char        *filename;
    int          useSystemDictionaries;
    GPtrArray   *dict_list;
    int          defaultUserDB;
};

TomoeConfig*
tomoe_config_new (const char* configFile)
{
    TomoeConfig* p;

    p           = calloc (1, sizeof (TomoeConfig));
    p->ref      = 1;
    p->dict_list = g_ptr_array_new ();

    /* check config file */
    if (configFile && 0 == access (configFile, F_OK | R_OK)) {
        p->filename = strdup (configFile);
    } else {
        /* use ~/.tomoe/config.xml */
        const char* home = getenv ("HOME");

        if (!home) {
            p->filename = NULL;
        } else {
            _tomoe_create_config_dir ();
            p->filename = calloc (strlen (home) + strlen ("/." PACKAGE) + strlen (defaultConfigFile) + 1, sizeof(char));
            sprintf (p->filename, "%s/.%s%s", home, PACKAGE, defaultConfigFile);
            /* if not found, use systemConfigFile */
            if (0 != access (p->filename, F_OK | R_OK) &&
                0 == access (systemConfigFile, F_OK | R_OK))
            {
                FILE *src_file, *dest_file;
                char buf[4096];
                size_t bytes;

                src_file = fopen (systemConfigFile, "r");
                dest_file = fopen (p->filename, "w");
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
    return p;
}

TomoeConfig*
tomoe_config_add_ref (TomoeConfig* t_config)
{
    if (!t_config) return NULL;
    t_config->ref ++;
    return t_config;
}

void
tomoe_config_free (TomoeConfig* t_config)
{
    if (!t_config) return;
    t_config->ref--;
    if (t_config->ref <= 0) {
        free (t_config->filename);
        TOMOE_PTR_ARRAY_FREE_ALL (t_config->dict_list, _tomoe_dict_cfg_free);
        free (t_config);
    }
}

void
tomoe_config_load (TomoeConfig* t_config)
{
    xmlDocPtr doc;
    xmlNodePtr root;
    char* defaultUserDB = NULL;
    int i;

    if (!t_config) return;
    doc = t_config->filename ? xmlReadFile (t_config->filename, NULL, 1)
                         : xmlReadDoc (defaultConfig, NULL, NULL, 1);

    root = xmlDocGetRootElement(doc);

    t_config->defaultUserDB = -1;
    t_config->useSystemDictionaries = 0;

    if (root && 0 == xmlStrcmp(root->name, BAD_CAST "tomoeConfig")) {
        xmlNodePtr node;

        for (node = root->children; node; node = node->next) {
            if (node->type != XML_ELEMENT_NODE)
                continue;

            if (0 == xmlStrcmp(node->name, BAD_CAST "useSystemDictionaries")) {
                t_config->useSystemDictionaries = 1;
            } else if (0 == xmlStrcmp(node->name, BAD_CAST "defaultUserDB")) {
                xmlAttrPtr prop;

                for (prop = node->properties; prop; prop = prop->next) {
                    if (0 == xmlStrcmp(prop->name, BAD_CAST "file"))
                        defaultUserDB = strdup ((const char*)prop->children->content);
                }
            } else if (0 == xmlStrcmp(node->name, BAD_CAST "dictionary")) {
                xmlAttrPtr prop;
                TomoeDictCfg* dcfg = _tomoe_dict_cfg_new ();

                dcfg->writeAccess = 0;
                dcfg->dontLoad = 0;
                dcfg->user = 1;
                for (prop = node->properties; prop; prop = prop->next) {
                    if (0 == xmlStrcmp(prop->name, BAD_CAST "file"))
                        dcfg->filename = strdup ((const char*)prop->children->content);
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
                    g_ptr_array_add (t_config->dict_list, dcfg);
                }
                /*
                else
                    {_tomoe_dict_cfg_free (dcfg);fprintf(stdout,"..not found\n");}
                */
            }

        }
    }
    xmlFreeDoc (doc);

    g_ptr_array_sort (t_config->dict_list, _tomoe_dict_cfg_cmp);

    if (defaultUserDB) {
        for (i = 0; i < t_config->dict_list->len; i++) {
            TomoeDictCfg* dcfg = (TomoeDictCfg*)g_ptr_array_index (t_config->dict_list, i);

            if (strcmp (defaultUserDB, dcfg->filename) == 0) {
                t_config->defaultUserDB = i;
                break;
            }
        }
    }

    /* search in TOMOEDATADIR for additional dictionaries */
    if (t_config->useSystemDictionaries) {
        size_t cnt;
        glob_t glob_results;
        char **p;

        glob (TOMOEDATADIR "/*.xml", 0, 0, &glob_results);

        for (p = glob_results.gl_pathv, cnt = glob_results.gl_pathc;
             cnt; p++, cnt--)
        {
            gchar *filename;
            gint i;
            gboolean dup = FALSE;

            filename = g_path_get_basename (*p);

            for (i = 0; i < t_config->dict_list->len; i++) {
                TomoeDictCfg *dcfg = g_ptr_array_index (t_config->dict_list, i);
                if (!strcmp (dcfg->filename, filename)) {
                    dup = TRUE;
                }
            }

            if (!dup ) {
                TomoeDictCfg* dcfg = _tomoe_dict_cfg_new ();
                dcfg->writeAccess = 0;
                dcfg->dontLoad = 0;
                dcfg->user = 0;
                dcfg->filename = filename;
                g_ptr_array_add (t_config->dict_list, dcfg);
            } else {
                g_free (filename);
            }
        }
        globfree (&glob_results);

        g_ptr_array_sort (t_config->dict_list, _tomoe_dict_cfg_cmp);
    }
}

void
tomoe_config_save (TomoeConfig *cfg)
{
    if (!cfg) return;
    if (cfg->filename) {
        xmlDocPtr doc;
        const char* param[3];
        xmlNodePtr root;
        int i;

        doc = xmlNewDoc(BAD_CAST "1.0");
        root = xmlNewNode(NULL, BAD_CAST "tomoeConfig");
        param[0] = 0;

        xmlDocSetRootElement (doc, root);

        if (cfg->useSystemDictionaries)
            xmlNewChild (root, NULL, BAD_CAST "useSystemDictionaries", NULL);


        for (i = 0; i < cfg->dict_list->len; i++) {
            xmlNodePtr node = xmlNewChild (root, NULL, BAD_CAST "dictionary", NULL);
            TomoeDictCfg *dict = (TomoeDictCfg*)g_ptr_array_index (cfg->dict_list, i);

            xmlNewProp (node, BAD_CAST "file", BAD_CAST dict->filename);
            if (!dict->user)
                xmlNewProp (node, BAD_CAST "system", BAD_CAST "yes");
            if (dict->dontLoad)
                xmlNewProp (node, BAD_CAST "dontLoad", BAD_CAST "yes");
        }

        xmlSaveFormatFileEnc(cfg->filename, doc, "UTF-8", 1);
        xmlFreeDoc (doc);
    }
}

const GPtrArray*
tomoe_config_get_dict_list (TomoeConfig* t_config)
{
    if (!t_config) return NULL;
    return t_config->dict_list;
}

int
tomoe_config_get_default_user_db (TomoeConfig *t_config)
{
    if (!t_config) return 0;
    return t_config->defaultUserDB;
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
    free (p->filename);
    free (p);
}

static int
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
    char *path;
    const char *home;

    home = getenv ("HOME");
    if (!home)
        return;

    path = calloc (strlen (home) + strlen ("/." PACKAGE), sizeof (char));
    sprintf (path, "%s/.%s", home, PACKAGE);

    if (access (path, F_OK | R_OK | W_OK) == 0) {
        free (path);
        return;
    }

    mkdir (path, 0711);
    free (path);
}

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
