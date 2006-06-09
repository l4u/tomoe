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
#include "array.h"

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
tomoe_db_add_dict (tomoe_db* this, const char *filename)
{
    tomoe_dict* dict;

    if (!this) return;
    if (!filename) return;

    dict = tomoe_dict_new (filename);
    tomoe_array_append (this->dicts, dict);
}

tomoe_array*
tomoe_db_get_matched (tomoe_db* this, tomoe_glyph* input)
{
    int i, num;
    tomoe_array* matched;
    tomoe_dict* dict;

    if (!this) return tomoe_array_new (NULL, NULL, NULL);
    num = tomoe_array_size (this->dicts);
    if (num == 0) return tomoe_array_new (NULL, NULL, NULL);

    dict = (tomoe_dict*)tomoe_array_get (this->dicts, 0);
    matched = tomoe_dict_get_matched (dict, input);
    for (i = 1; i < num; i++)
    {
        tomoe_array* tmp;

        dict = (tomoe_dict*)tomoe_array_get (this->dicts, i);
        tmp = tomoe_dict_get_matched (dict, input);
        tomoe_array_merge (matched, tmp);
        tomoe_array_free (tmp);
    }
    tomoe_array_sort (matched);

    return matched;
}

tomoe_array*
tomoe_db_get_reading (tomoe_db* this, const char* input)
{
    tomoe_array* p = tomoe_array_new (NULL, NULL, NULL);
    return p;
}
