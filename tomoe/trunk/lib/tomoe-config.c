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
#include <glob.h>

#include "tomoe-config.h"

static const xmlChar* defaultConfig  = BAD_CAST "<?xml version=\"1.0\" standalone=\"no\"?>" \
                                "	<tomoeConfig>" \
                                "	<useSystemDictionaries value=\"yes\"/>" \
                                "</tomoeConfig>";

static const char* systemConfigFile = TOMOESYSCONFDIR "/config.xml";
static const char* defaultConfigFile = "/config.xml";

static tomoe_dict_cfg* _tomoe_dict_cfg_new      (void);
static void            _tomoe_dict_cfg_free     (tomoe_dict_cfg* p);
static int             _tomoe_dict_cfg_cmp      (const tomoe_dict_cfg** a, const tomoe_dict_cfg** b);
static void            _tomoe_create_config_dir (void);

struct _tomoe_config
{
    int          ref;
    char*        filename;
    int          useSystemDictionaries;
    tomoe_array* dictList;
    int          defaultUserDB;
};

tomoe_config*
tomoe_config_new (const char* configFile)
{
    tomoe_config* p;
    p           = calloc (1, sizeof (tomoe_config));
    p->ref      = 1;
    p->dictList = tomoe_array_new ((tomoe_compare_fn)_tomoe_dict_cfg_cmp,
                                   NULL,
                                   (tomoe_free_fn)_tomoe_dict_cfg_free);

    /* check config file */
    if (configFile && 0 == access (configFile, F_OK | R_OK))
        p->filename = strdup (configFile);
    else
    {
        /* use ~/.tomoe/config.xml */
        const char* home = getenv ("HOME");
        if (!home)
            p->filename = NULL;
        else
        {
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

tomoe_config*
tomoe_config_add_ref (tomoe_config* this)
{
    if (!this) return NULL;
    this->ref ++;
    return this;
}

void
tomoe_config_free (tomoe_config* this)
{
    if (!this) return;
    this->ref--;
    if (this->ref <= 0)
    {
        free (this->filename);
        tomoe_array_free (this->dictList);
        free (this);
    }
}

void
tomoe_config_load (tomoe_config* this)
{
    xmlDocPtr doc;
    xmlNodePtr root;
    char* defaultUserDB = NULL;
    int i;

    if (!this) return;
    doc = this->filename ? xmlReadFile (this->filename, NULL, 1)
                         : xmlReadDoc (defaultConfig, NULL, NULL, 1);

    root = xmlDocGetRootElement(doc);

    this->defaultUserDB = -1;
    this->useSystemDictionaries = 0;

    if (root && 0 == xmlStrcmp(root->name, BAD_CAST "tomoeConfig"))
    {
        xmlNodePtr node;
        for (node = root->children; node; node = node->next)
        {
            if (node->type != XML_ELEMENT_NODE)
                continue;

            if (0 == xmlStrcmp(node->name, BAD_CAST "useSystemDictionaries"))
            {
                this->useSystemDictionaries = 1;
            }
            else if (0 == xmlStrcmp(node->name, BAD_CAST "defaultUserDB"))
            {
                xmlAttrPtr prop;
                for (prop = node->properties; prop; prop = prop->next)
                {
                    if (0 == xmlStrcmp(prop->name, BAD_CAST "file"))
                        defaultUserDB = strdup ((const char*)prop->children->content);
                }
            }
            else if (0 == xmlStrcmp(node->name, BAD_CAST "dictionary"))
            {
                xmlAttrPtr prop;
                tomoe_dict_cfg* dcfg = _tomoe_dict_cfg_new ();
                dcfg->writeAccess = 0;
                dcfg->dontLoad = 0;
                dcfg->user = 1;
                for (prop = node->properties; prop; prop = prop->next)
                {
                    if (0 == xmlStrcmp(prop->name, BAD_CAST "file"))
                        dcfg->filename = strdup ((const char*)prop->children->content);
                    else if (0 == xmlStrcmp(prop->name, BAD_CAST "dontLoad"))
                        dcfg->dontLoad = xmlStrcmp(prop->children->content, BAD_CAST "yes") ? 0 : 1;
                    else if (0 == xmlStrcmp(prop->name, BAD_CAST "system"))
                        dcfg->user = xmlStrcmp(prop->children->content, BAD_CAST "yes") ? 1 : 0;
                }

                /* check if file exists */fprintf (stdout, dcfg->filename);
                /*if (access (dcfg->filename, F_OK | R_OK)) FIXME*/
                {/*fprintf(stdout, "..access ok\n");*/
                    dcfg->writeAccess = /*access (dcfg->filename, W_OK) ? */dcfg->user/* : 0*/;
                    tomoe_array_append (this->dictList, dcfg);
                }
                /*
                else
                    {_tomoe_dict_cfg_free (dcfg);fprintf(stdout,"..not found\n");}
                */
            }

        }
    }
    xmlFreeDoc (doc);

    tomoe_array_sort (this->dictList);

    if (defaultUserDB)
    {
        for (i = 0; i < tomoe_array_size (this->dictList); i++)
        {
            tomoe_dict_cfg* dcfg = (tomoe_dict_cfg*)tomoe_array_get (this->dictList, i);
            if (strcmp (defaultUserDB, dcfg->filename) == 0)
            {
                this->defaultUserDB = i;
                break;
            }
        }
    }

    /* search in TOMOEDATADIR for additional dictionaries */
    if (this->useSystemDictionaries)
    {
        tomoe_array* systemList = tomoe_array_new (NULL, NULL, NULL);
        size_t cnt;
        glob_t glob_results;
        char **p;

        glob (TOMOEDATADIR "/*.xml", 0, 0, &glob_results);

        for (p = glob_results.gl_pathv, cnt = glob_results.gl_pathc;
             cnt; p++, cnt--)
        {
            tomoe_dict_cfg* dcfg = _tomoe_dict_cfg_new ();
            char* slash;
            dcfg->writeAccess = 0;
            dcfg->dontLoad = 0;
            dcfg->user = 0;
            slash = strrchr ( *p, '/');
            if (slash)
            {
                dcfg->filename = strdup (slash + 1);
                if (-1 == tomoe_array_find (this->dictList, dcfg))
                {
                    tomoe_array_append (systemList, dcfg);
                    continue;
                }
            }

            _tomoe_dict_cfg_free (dcfg);
        }
        globfree (&glob_results);

        tomoe_array_merge (this->dictList, systemList);
        tomoe_array_sort (this->dictList);
        tomoe_array_free (systemList);
    }
}

void
tomoe_config_save (tomoe_config *cfg)
{
    if (!cfg) return;
    if (cfg->filename)
    {
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


        for (i = 0; i < tomoe_array_size (cfg->dictList); i++)
        {
            xmlNodePtr node = xmlNewChild (root, NULL, BAD_CAST "dictionary", NULL);
            tomoe_dict_cfg *dict = (tomoe_dict_cfg*)tomoe_array_get (cfg->dictList, i);

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

tomoe_array*
tomoe_config_get_dict_list (tomoe_config* this)
{
    if (!this) return NULL;
    return tomoe_array_add_ref (this->dictList);
}

int
tomoe_config_get_default_user_db (tomoe_config *this)
{
    if (!this) return 0;
    return this->defaultUserDB;
}

static tomoe_dict_cfg*
_tomoe_dict_cfg_new (void)
{
    tomoe_dict_cfg* p = (tomoe_dict_cfg*)calloc (1, sizeof (tomoe_dict_cfg));
    p->filename = NULL;
    return p;
}

static void
_tomoe_dict_cfg_free (tomoe_dict_cfg* p)
{
    if (!p) return;
    free (p->filename);
    free (p);
}

static int
_tomoe_dict_cfg_cmp (const tomoe_dict_cfg** a, const tomoe_dict_cfg** b)
{
    return (*a)->user == (*b)->user ? strcmp((*a)->filename, (*b)->filename) 
                                    : (*b)->user - (*a)->user;
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

    if (access (path, F_OK | R_OK | W_OK) == 0)
    {
        free (path);
        return;
    }

    mkdir (path, 0711);
    free (path);
}

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
