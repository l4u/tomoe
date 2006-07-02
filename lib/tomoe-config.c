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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlreader.h>
#include <glob.h>

#include "tomoe-config.h"

const xmlChar* defaultConfig  = BAD_CAST "<?xml version=\"1.0\" standalone=\"no\"?>" \
                                "	<tomoeConfig>" \
                                "	<useSystemDictionaries value=\"yes\"/>" \
                                "</tomoeConfig>";
const char* defaultConfigFile = "/.tomoe-config.xml";

tomoe_dict_cfg* _tomoe_dict_cfg_new  ();
void            _tomoe_dict_cfg_free (tomoe_dict_cfg* p);
int             _tomoe_dict_cfg_cmp  (const tomoe_dict_cfg** a, const tomoe_dict_cfg** b);

struct _tomoe_config
{
    int          ref;
    char*        filename;
    int          useSystemDictionaries;
    tomoe_array* dictList;
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

    if (configFile)
        p->filename = strdup (configFile);
    else
    {
        // use ~/.tomoe-config.xml
        const char* home = getenv ("HOME");
        if (!home)
            p->filename = NULL;
        else
        {
            p->filename = calloc (strlen (home) + strlen (defaultConfigFile) + 1, sizeof(char));
            strcpy (p->filename, home);
            strcat (p->filename, defaultConfigFile);
        }
    }
    return p;
}

tomoe_config*
tomoe_config_addref (tomoe_config* this)
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
    int system = 0;

    if (!this) return;
    doc = this->filename ? xmlReadFile (this->filename, NULL, 1)
                         : xmlReadDoc ( defaultConfig, NULL, NULL, 1);
    root = xmlDocGetRootElement(doc);

    if (root && 0 == xmlStrcmp(root->name, BAD_CAST "tomoeConfig"))
    {
        xmlNodePtr node;
        for (node = root->children; node; node = node->next)
        {
            if (node->type != XML_ELEMENT_NODE)
                continue;

            if (0 == xmlStrcmp(node->name, BAD_CAST "useSystemDictionaries"))
            {
                system = 1;
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
                dcfg->writeAccess = dcfg->user; //FIXME check rights
                tomoe_array_append (this->dictList, dcfg);
            }

        }
    }
    xmlFreeDoc (doc);

    tomoe_array_sort (this->dictList);

    // search in TOMOEDATADIR for additional dictionaries
    if (system)
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
tomoe_config_save (tomoe_config* this)
{
    if (!this) return;
    if (this->filename)
    {
        // TODO
    }
}

tomoe_array*
tomoe_config_getDictList (tomoe_config* this)
{
    if (!this) return NULL;
    return tomoe_array_addref (this->dictList);
}

tomoe_dict_cfg*
_tomoe_dict_cfg_new ()
{
    tomoe_dict_cfg* p = (tomoe_dict_cfg*)calloc (1, sizeof (tomoe_dict_cfg));
    p->filename = NULL;
    return p;
}

void
_tomoe_dict_cfg_free (tomoe_dict_cfg* p)
{
    if (!p) return;
    free (p->filename);
    free (p);
}

int
_tomoe_dict_cfg_cmp (const tomoe_dict_cfg** a, const tomoe_dict_cfg** b)
{
    return (*a)->user == (*b)->user ? strcmp((*a)->filename, (*b)->filename) 
                                    : (*a)->user - (*b)->user;
}
