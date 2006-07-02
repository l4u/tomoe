/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2006 Juernjakob Harder <juernjakob@gmail.com>
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

#ifndef __TOMOE_ARRAY_H__
#define __TOMOE_ARRAY_H__

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct _int_array int_array;

struct _int_array
{
    int *p;
    int  len;
    int  ref_count;
};

int_array *_int_array_new         (void);
int_array *_int_array_append_data (int_array *a, int i);
int_array *_int_array_ref         (int_array *a);
int_array *_int_array_copy        (int_array *a);
int        _int_array_find_data   (int_array *a, int i);

void       _int_array_unref       (int_array *a);

typedef struct _pointer_array pointer_array;

struct _pointer_array
{
    void **p;
    int    len;
    int    ref_count;
};

pointer_array *_pointer_array_new         (void);
pointer_array *_pointer_array_append_data (pointer_array *a, void *p);
pointer_array *_pointer_array_ref         (pointer_array *a);
int            _pointer_array_find_data   (pointer_array *a, void *p);

void           _pointer_array_unref       (pointer_array *a);

typedef struct _tomoe_array tomoe_array;

typedef void* (*tomoe_addref_fn)          (void*);
typedef void  (*tomoe_free_fn)            (void*);
typedef int   (*tomoe_compare_fn)         (const void**,
                                           const void**);

tomoe_array*   tomoe_array_new            (tomoe_compare_fn  compare,
                                           tomoe_addref_fn   addref,
                                           tomoe_free_fn     free);
tomoe_array*   tomoe_array_addref         (tomoe_array*      this);
void           tomoe_array_free           (tomoe_array*      this);

tomoe_array*   tomoe_array_append         (tomoe_array*      this,
                                           void*             p);
int            tomoe_array_find           (tomoe_array*      this,
                                           void*             p);
void*          tomoe_array_get            (tomoe_array*      this,
                                           int               index);
void           tomoe_array_remove         (tomoe_array*      this,
                                           int               index);

void           tomoe_array_sort           (tomoe_array*      this);
int            tomoe_array_size           (tomoe_array*      this);
void           tomoe_array_merge          (tomoe_array*      this,
                                           tomoe_array*      append);

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_ARRAY_H__ */
