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

#include "tomoe-dict.h"
#include "tomoe-db.h"
#include "tomoe-array.h"

struct _tomoe_db
{
    int          ref;
    tomoe_array* dicts;
};


tomoe_db*
tomoe_db_new(void)
{
    tomoe_db* p;
    p        = calloc (1, sizeof(tomoe_db));
    p->ref   = 1;
    p->dicts = tomoe_array_new (NULL,
                                (tomoe_addref_fn)tomoe_dict_addref,
                                (tomoe_free_fn)tomoe_dict_free);
    return p;
}

tomoe_db*
tomoe_db_addref(tomoe_db* this)
{
    if (!this) return NULL;
    this->ref ++;
    return this;
}

void
tomoe_db_free(tomoe_db* this)
{
    if (!this) return;
    this->ref--;
    if (this->ref <= 0)
    {
        tomoe_array_free (this->dicts);
        free (this);
    }
}

void
tomoe_db_addDict (tomoe_db* this, tomoe_dict* dict)
{
    if (!this || !dict) return;
    tomoe_array_append (this->dicts, tomoe_dict_addref (dict));
}

void
tomoe_db_loadDict (tomoe_db* this, const char *filename, int editable)
{
    tomoe_dict* dict;

    if (!this) return;
    if (!filename) return;

    fprintf (stdout, "load dictionary '%s' editable: %s...", filename, editable ? "yes" : "no");
    fflush (stdout);
    dict = tomoe_dict_new (filename, editable);
    if (dict)
        tomoe_array_append (this->dicts, dict);
    printf (" ok\n");
}

void
tomoe_db_loadDictList (tomoe_db* this, tomoe_array* list)
{
    int i;
    for (i = 0; i < tomoe_array_size (list); i++)
    {
        tomoe_dict_cfg* p = tomoe_array_get (list, i);
        if (p->dontLoad) continue;

        if (p->user)
            tomoe_db_loadDict (this, p->filename, p->writeAccess);
        else
        {
            char* file = calloc (strlen (p->filename) + strlen (TOMOEDATADIR) + 2, sizeof (char));
            strcpy (file, TOMOEDATADIR);
            strcat (file, "/");
            strcat (file, p->filename);
            tomoe_db_loadDict (this, file, p->writeAccess);
        }
    }
}

tomoe_array*
tomoe_db_getDictList (tomoe_db* this)
{
    if (!this) return NULL;
    return tomoe_array_addref(this->dicts);
}

void
tomoe_db_save (tomoe_db *db)
{
    int i;

    if (!db) return;

    for (i = 0; i < tomoe_array_size (db->dicts); i++)
    {
        tomoe_dict *dict = (tomoe_dict*)tomoe_array_get (db->dicts, i);
        if (tomoe_dict_get_modified (dict))
            tomoe_dict_save (dict);
    }
}

tomoe_array*
tomoe_db_searchByStrokes (tomoe_db* this, tomoe_glyph* input)
{
    int i, num;
    tomoe_array* tmp;
    tomoe_array* matched;
    tomoe_dict* dict;

    if (!this) return tomoe_array_new (NULL, NULL, NULL);
    num = tomoe_array_size (this->dicts);
    if (num == 0) return tomoe_array_new (NULL, NULL, NULL);

    dict = (tomoe_dict*)tomoe_array_get (this->dicts, 0);
    tmp = tomoe_dict_searchByStrokes (dict, input);
    matched = tomoe_array_cloneEmpty (tmp);
    for (i = 0; i < num; i++)
    {
        tomoe_array* tmp;
        dict = (tomoe_dict*)tomoe_array_get (this->dicts, i);
        tmp = tomoe_dict_searchByStrokes (dict, input);
        tomoe_array_merge (matched, tmp);
        tomoe_array_free (tmp);
    }
    tomoe_array_sort (matched);

    return matched;
}

tomoe_array*
tomoe_db_searchByReading (tomoe_db* this, const char* input)
{
    int i, num;
    tomoe_array* reading;
    tomoe_array* tmp;
    tomoe_dict* dict;

    if (!this) return tomoe_array_new (NULL, NULL, NULL);
    num = tomoe_array_size (this->dicts);
    if (num == 0) return tomoe_array_new (NULL, NULL, NULL);

    dict = (tomoe_dict*)tomoe_array_get (this->dicts, 0);
    tmp = tomoe_dict_searchByReading (dict, input);
    reading = tomoe_array_cloneEmpty (tmp);
    for (i = 0; i < num; i++)
    {
        dict = (tomoe_dict*)tomoe_array_get (this->dicts, i);
        tmp = tomoe_dict_searchByReading (dict, input);
        tomoe_array_merge (reading, tmp);
        tomoe_array_free (tmp);
    }
    //tomoe_array_sort (reading);

    return reading;
}
