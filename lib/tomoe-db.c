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

struct _TomoeDB
{
    int         ref;
    TomoeArray *dicts;
};


TomoeDB*
tomoe_db_new(void)
{
    TomoeDB* p;
    p        = calloc (1, sizeof(TomoeDB));
    p->ref   = 1;
    p->dicts = tomoe_array_new (NULL,
                                (tomoe_addref_fn)tomoe_dict_add_ref,
                                (tomoe_free_fn)tomoe_dict_free);
    return p;
}

TomoeDB*
tomoe_db_add_ref(TomoeDB* t_db)
{
    if (!t_db) return NULL;
    t_db->ref ++;
    return t_db;
}

void
tomoe_db_free(TomoeDB* t_db)
{
    if (!t_db) return;
    t_db->ref--;
    if (t_db->ref <= 0)
    {
        tomoe_array_free (t_db->dicts);
        free (t_db);
    }
}

void
tomoe_db_add_dict (TomoeDB* t_db, TomoeDict* dict)
{
    if (!t_db || !dict) return;
    tomoe_array_append (t_db->dicts, tomoe_dict_add_ref (dict));
}

void
tomoe_db_load_dict (TomoeDB* t_db, const char *filename, int editable)
{
    TomoeDict* dict;

    if (!t_db) return;
    if (!filename) return;

    fprintf (stdout, "load dictionary '%s' editable: %s...", filename, editable ? "yes" : "no");
    fflush (stdout);
    dict = tomoe_dict_new (filename, editable);
    if (dict)
        tomoe_array_append (t_db->dicts, dict);
    printf (" ok\n");
}

void
tomoe_db_load_dict_list (TomoeDB* t_db, TomoeArray* list)
{
    int i;
    for (i = 0; i < tomoe_array_size (list); i++)
    {
        tomoe_dict_cfg* p = tomoe_array_get (list, i);
        if (p->dontLoad) continue;

        if (p->user)
            tomoe_db_load_dict (t_db, p->filename, p->writeAccess);
        else
        {
            char* file = calloc (strlen (p->filename) + strlen (TOMOEDATADIR) + 2, sizeof (char));
            strcpy (file, TOMOEDATADIR);
            strcat (file, "/");
            strcat (file, p->filename);
            tomoe_db_load_dict (t_db, file, p->writeAccess);
        }
    }
}

TomoeArray*
tomoe_db_get_dict_list (TomoeDB* t_db)
{
    if (!t_db) return NULL;
    return tomoe_array_add_ref(t_db->dicts);
}

void
tomoe_db_save (TomoeDB *db)
{
    int i;

    if (!db) return;

    for (i = 0; i < tomoe_array_size (db->dicts); i++)
    {
        TomoeDict *dict = (TomoeDict*)tomoe_array_get (db->dicts, i);
        if (tomoe_dict_is_modified (dict))
            tomoe_dict_save (dict);
    }
}

TomoeArray*
tomoe_db_search_by_strokes (TomoeDB* t_db, TomoeGlyph* input)
{
    int i, num;
    TomoeArray* tmp;
    TomoeArray* matched;
    TomoeDict* dict;

    if (!t_db) return tomoe_array_new (NULL, NULL, NULL);
    num = tomoe_array_size (t_db->dicts);
    if (num == 0) return tomoe_array_new (NULL, NULL, NULL);

    dict = (TomoeDict*)tomoe_array_get (t_db->dicts, 0);
    tmp = tomoe_dict_search_by_strokes (dict, input);
    matched = tomoe_array_clone_empty (tmp);
    for (i = 0; i < num; i++)
    {
        TomoeArray* tmp;
        dict = (TomoeDict*)tomoe_array_get (t_db->dicts, i);
        tmp = tomoe_dict_search_by_strokes (dict, input);
        tomoe_array_merge (matched, tmp);
        tomoe_array_free (tmp);
    }
    tomoe_array_sort (matched);

    return matched;
}

TomoeArray*
tomoe_db_search_by_reading (TomoeDB* t_db, const char* input)
{
    int i, num;
    TomoeArray* reading;
    TomoeArray* tmp;
    TomoeDict*   dict;

    if (!t_db) return tomoe_array_new (NULL, NULL, NULL);
    num = tomoe_array_size (t_db->dicts);
    if (num == 0) return tomoe_array_new (NULL, NULL, NULL);

    dict = (TomoeDict*)tomoe_array_get (t_db->dicts, 0);
    tmp = tomoe_dict_search_by_reading (dict, input);
    reading = tomoe_array_clone_empty (tmp);
    for (i = 0; i < num; i++)
    {
        dict = (TomoeDict*)tomoe_array_get (t_db->dicts, i);
        tmp = tomoe_dict_search_by_reading (dict, input);
        tomoe_array_merge (reading, tmp);
        tomoe_array_free (tmp);
    }
    /*tomoe_array_sort (reading);*/

    return reading;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
