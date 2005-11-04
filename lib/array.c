/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
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

#include "array.h"
#include "stdlib.h"
#include "stdio.h"


int_array *
_int_array_new (void)
{
    int_array *a;

    a = calloc (sizeof (int_array), 1);
    a->len = 0;
    a->p = NULL;
    a->ref_count = 1;

    return a;
}

int_array *
_int_array_append_data (int_array *a, int i)
{
    if (!a)
        return NULL;
  
    a->len++;
    a->p = realloc (a->p, sizeof (int) * (a->len));
    a->p[a->len - 1] = i;

    return a;
}

int_array *
_int_array_copy (int_array *a)
{
    int i;
    int_array *ret;

    ret = _int_array_new ();

    ret->len = a->len;

    ret->p = calloc (ret->len, sizeof (int));
 
    for (i = 0; i < ret->len; i++)
    {
        ret->p[i] = a->p[i];
    }

    return ret;
}

int_array *
_int_array_ref (int_array *a)
{
    if (!a)
        return NULL;
    a->ref_count++;

    return a;
}

void
_int_array_unref (int_array *a)
{
    if (!a)
        return;
 
    a->ref_count--;
    if (a->ref_count == 0)
    {
        if (a->p)
            free (a->p);
        a->p = NULL;

        free (a);
    }
}

int
_int_array_find_data (int_array *a, int i)
{
    int l;

    if (!a || a->len == 0)
        return -1;
  
    for (l = 0; l < a->len; l++)
    {
        if (a->p[l] == i)
            return l;
    }

    return -1;
}

pointer_array *
_pointer_array_new (void)
{
    pointer_array *a;

    a = calloc (sizeof (pointer_array), 1);
    a->len = 0;
    a->p = NULL;
    a->ref_count = 1;

    return a;
}

pointer_array *
_pointer_array_append_data (pointer_array *a, void *p)
{
    if (!a)
        return NULL;
  
    a->len++;
    a->p = realloc (a->p, sizeof (void*) * (a->len));
    a->p[a->len - 1] = p;

    return a;
}

pointer_array *
_pointer_array_ref (pointer_array *a)
{
    if (!a)
        return NULL;
    a->ref_count++;

    return a;
}

void
_pointer_array_unref (pointer_array *a)
{
    if (!a)
        return;
 
    a->ref_count--;
    if (a->ref_count == 0)
    {
        if (a->p)
            free (a->p);
        a->p = NULL;

        free (a);
    }
}

int
_pointer_array_find_data (pointer_array *a, void *p)
{
    int l;
    if (!a)
      return -1;
  
    for (l = 0; l < a->len; l++)
    {
        if (a->p[l] == p)
            return l;
    }

    return -1;
}

/*
vi:ts=2:nowrap:ai:expandtab
*/
