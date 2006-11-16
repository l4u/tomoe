/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
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

#include "tomoe-array.h"
#include "stdlib.h"
#include "stdio.h"

struct _TomoeArray
{
    int                ref;
    void**             p;
    int                len;
    int                cap;
    tomoe_compare_fn   compare;
    tomoe_addref_fn    addref;
    tomoe_free_fn      free;
};

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

TomoeArray*
tomoe_array_new (tomoe_compare_fn  compare,
                 tomoe_addref_fn   addref,
                 tomoe_free_fn     free)
{
    TomoeArray* p;

    p           = calloc(1, sizeof(TomoeArray));
    p->ref      = 1;
    p->len      = 0;
    p->cap      = 32;
    p->p        = calloc(32, sizeof(void*));
    p->compare  = (tomoe_compare_fn)compare;
    p->addref   = (tomoe_addref_fn)addref;
    p->free     = (tomoe_free_fn)free;

    return p;
}

TomoeArray*
tomoe_array_add_ref(TomoeArray* t_array)
{
    if (!t_array) return 0;
    t_array->ref ++;
    return t_array;
}

void
tomoe_array_free(TomoeArray* t_array)
{
    int i;

    if (!t_array) return;

    t_array->ref --;
    if (t_array->ref <= 0)
    {
        if (t_array->free)
            for (i = 0; i < t_array->len; i++)
                t_array->free(t_array->p[i]);
        free(t_array);
    }
}

TomoeArray*
tomoe_array_append (TomoeArray* t_array, void* p)
{
    if (!t_array) return NULL;

    if (t_array->len == t_array->cap)
    {
        t_array->cap += t_array->cap / 2;
        t_array->p = realloc (t_array->p, sizeof (void*) * (t_array->cap));
    }

    if (!t_array->addref)
        t_array->p[t_array->len] = p;
    else
        t_array->p[t_array->len] = t_array->addref(p);
    t_array->len ++;

    return t_array;
}

int
tomoe_array_find (const TomoeArray* t_array, const void* p)
{
    void* e;
    if (!t_array || !t_array->compare) return -1;

    e = bsearch(&p, t_array->p, t_array->len, sizeof(void*),
        ((int(*)(const void*, const void*))t_array->compare));
    if (!e) return -1;

    return (((int*)e) - ((int*)t_array->p));
}

void*
tomoe_array_get (TomoeArray* t_array, int index)
{
    if (!t_array || index < 0 || t_array->len <= index) return NULL;
    return t_array->p[index];
}

const void*
tomoe_array_get_const (const TomoeArray* t_array, int index)
{
    if (!t_array || index < 0 || t_array->len <= index) return NULL;
    return t_array->p[index];
}

void
tomoe_array_remove (TomoeArray* t_array, int index)
{
    int i;
    if (!t_array || index < 0 || t_array->len <= index) return;
    if (t_array->free)
        t_array->free (t_array->p[index]);
    for (i = index + 1; i < t_array->len; i++)
        t_array->p[i - 1] = t_array->p[i];
    t_array->len --;
}

void
tomoe_array_sort (TomoeArray* t_array)
{
    if (!t_array || !t_array->compare) return;

    qsort(t_array->p, t_array->len, sizeof(void*),
        ((int(*)(const void*, const void*))t_array->compare));
}

int
tomoe_array_size (const TomoeArray* t_array)
{
    if (!t_array) return 0;
    return t_array->len;
}

void
tomoe_array_merge (TomoeArray* t_array, TomoeArray* append)
{
    int i, num;

    if (!t_array || !append) return;
    num = tomoe_array_size (append);
    for (i = 0; i < num; i++)
        tomoe_array_append (t_array, tomoe_array_get (append, i));
}

TomoeArray*
tomoe_array_clone_empty (const TomoeArray* t_array)
{
    if (!t_array) return NULL;
    return tomoe_array_new (t_array->compare, t_array->addref, t_array->free);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
