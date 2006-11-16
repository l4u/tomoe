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

/** @file tomoe-array.h
 *  @brief Provides array utilities.
 */

#ifndef __TOMOE_ARRAY_H__
#define __TOMOE_ARRAY_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* TODO remove from tomoe, replace by TomoeArray */
typedef struct _int_array int_array;

struct _int_array
{
    int *p;
    int  len;
    int  ref_count;
};

int_array*      _int_array_new                  (void);
int_array*      _int_array_append_data          (int_array *a, int i);
int_array*      _int_array_ref                  (int_array *a);
int_array*      _int_array_copy                 (int_array *a);
int             _int_array_find_data            (int_array *a, int i);

void            _int_array_unref                (int_array *a);

/* TODO remove from tomoe, replace by TomoeArray */
typedef struct _pointer_array pointer_array;

struct _pointer_array
{
    void **p;
    int    len;
    int    ref_count;
};

pointer_array*  _pointer_array_new              (void);
pointer_array*  _pointer_array_append_data      (pointer_array *a, void *p);
pointer_array*  _pointer_array_ref              (pointer_array *a);
int             _pointer_array_find_data        (pointer_array *a, void *p);

void            _pointer_array_unref            (pointer_array *a);

/**
 * @typedef typedef struct _TomoeArray TomoeArray;
 *
 * A struct type which represents TomoeArray. All members in it should be
 * accessed through TomoeArray_* functions.
 */
typedef struct _TomoeArray TomoeArray;

/**
 * @typedef typedef void* (*tomoe_addref_fn) (void*);
 *
 * A function pointer type representing the addReference method of an object.
 */
typedef void*   (*tomoe_addref_fn)              (void*);

/**
 * @typedef typedef void* (*tomoe_free_fn) (void*);
 *
 * A function pointer type representing the free method of an object.
 */
typedef void    (*tomoe_free_fn)                (void*);

/**
 * @typedef typedef void* (*tomoe_compare_fn) (void*);
 *
 * A function pointer type representing the compare method of an object.
 */
typedef int     (*tomoe_compare_fn)             (const void**,
                                                 const void**);

/**
 * @brief Create a new array object.
 * @param compare - Pointer to the compare method of the array object type.
 * @param addref  - Pointer to the addReference method of the array object type.
 * @param free    - Pointer to the free method of the array object type.
 * @return Pointer to newly allocated TomoeArray object.
 */
TomoeArray*     tomoe_array_new                 (tomoe_compare_fn   compare,
                                                 tomoe_addref_fn    addref,
                                                 tomoe_free_fn      free);

/**
 * @brief Increase reference count.
 * @param t_array - Pointer to the TomoeArray.
 * @return The TomoeArray.
 */
TomoeArray*     tomoe_array_add_ref             (TomoeArray        *t_array);

/**
 * @brief Decrease reference count and free if zero.
 * @param t_array - Pointer to the TomoeArray.
 */
void            tomoe_array_free                (TomoeArray       *t_array);

/**
 * @brief Append an object to the array.
 * @param t_array - Pointer to the TomoeArray.
 * @param p       - Pointer to the object to append.
 * @return The TomoeArray.
 */
TomoeArray*     tomoe_array_append              (TomoeArray        *t_array,
                                                 void              *p);

/**
 * @brief Find the index of an object by search key of tomoe_compare_fn.
 * @param t_array - Pointer to the TomoeArray.
 * @param p       - Pointer to the object to find.
 * @return Index of the element, -1 if not found.
 */
int             tomoe_array_find                (const TomoeArray  *t_array,
                                                 const void        *p);

/**
 * @brief Return an object element.
 * @param t_array - Pointer to the TomoeArray.
 * @param index   - Index of the element.
 * @return Object, NULL if index is out of range.
 */
void*           tomoe_array_get                 (TomoeArray        *t_array,
                                                 int                index);

/**
 * @brief Return an object element from a const array.
 * @param t_array - Pointer to the TomoeArray.
 * @param index   - Index of the element.
 * @return Object, NULL if index is out of range.
 */
const void*     tomoe_array_get_const           (const TomoeArray *t_array,
                                                 int                index);

/**
 * @brief Remove an element.
 * @param t_array - Pointer to the TomoeArray.
 * @param index   - Index of the element.
 */
void            tomoe_array_remove              (TomoeArray        *t_array,
                                                 int                index);

/**
 * @brief QuickSort the array by tomoe_compare_fn.
 * @param t_array - Pointer to the TomoeArray.
 */
void            tomoe_array_sort                (TomoeArray        *t_array);

/**
 * @brief Return the number of elements in the array.
 * @param t_array - Pointer to the TomoeArray.
 * @return Object element count.
 */
int             tomoe_array_size                (const TomoeArray  *t_array);

/**
 * @brief Append the content of another array to t_array.
 * @param t_array - Pointer to the TomoeArray.
 * @param append  - Pointer to the TomoeArray to append.
 */
void            tomoe_array_merge               (TomoeArray        *t_array,
                                                 TomoeArray        *append);

/**
 * @brief Create an empty array with same compare, addref, free functions.
 * @param t_array - Pointer to the TomoeArray.
 * @return Pointer to the newly allocated TomoeArray.
 */
TomoeArray*     tomoe_array_clone_empty         (const TomoeArray *t_array);

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_ARRAY_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
