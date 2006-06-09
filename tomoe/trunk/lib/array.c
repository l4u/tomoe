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

#include "array.h"
#include "stdlib.h"
#include "stdio.h"

struct _tomoe_array
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

tomoe_array*
tomoe_array_new (tomoe_compare_fn  compare,
                 tomoe_addref_fn   addref,
                 tomoe_free_fn     free)
{
    tomoe_array* p;

    p           = calloc(1, sizeof(tomoe_array));
    p->ref      = 1;
    p->len      = 0;
    p->cap      = 32;
    p->p        = calloc(32, sizeof(void*));
    p->compare  = (tomoe_compare_fn)compare;
    p->addref   = (tomoe_addref_fn)addref;
    p->free     = (tomoe_free_fn)free;

    return p;
}

tomoe_array*
tomoe_array_addref(tomoe_array* this)
{
    if (!this) return 0;
    this->ref ++;
    return this;
}

void
tomoe_array_free(tomoe_array* this)
{
    int i;

    if (!this) return;

    this->ref --;
    if (this->ref <= 0)
    {
        if (!this->free)
            for (i = 0; i < this->len; i++)
                this->free(this->p[i]);
        free(this);
    }
}

tomoe_array*
tomoe_array_append (tomoe_array* this, void* p)
{
    if (!this) return 0;

    if (this->len == this->cap)
    {
        this->cap += this->cap / 2;
        this->p = realloc (this->p, sizeof (void*) * (this->cap));
    }

    if (!this->addref)
        this->p[this->len] = p;
    else
        this->p[this->len] = this->addref(p);
    this->len ++;

    return this;
}

int
tomoe_array_find (tomoe_array* this, void* p)
{
    void* e;
    if (!this || !this->compare) return -1;

    e = bsearch(&p, this->p, this->len, sizeof(void*), 
        ((int(*)(const void*, const void*))this->compare));
    if (!e) return -1;

    return (((int*)e) - ((int*)p)) / sizeof(int*);
}

void
tomoe_array_sort (tomoe_array* this)
{
    if (!this || !this->compare) return;

    qsort(this->p, this->len, sizeof(void*), 
        ((int(*)(const void*, const void*))this->compare));
}

int
tomoe_array_size (tomoe_array* this)
{
    if (!this) return 0;
    return this->len;
}

void*
tomoe_array_get (tomoe_array* this, int index)
{
    if (!this || index < 0 || this->len <= index) return NULL;
    return this->p[index];
}

void
tomoe_array_merge (tomoe_array* this, tomoe_array* append)
{
    int i, num;

    if (!this || !append) return;
    num = tomoe_array_size(append);
    for (i = 0; i < num; i++)
        tomoe_array_append(this, tomoe_array_get(append, i));
}

/*
vi:ts=2:nowrap:ai:expandtab
*/
