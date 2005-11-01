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

#ifndef __ARRAY_H__
#define __ARRAY_H__

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct _int_array int_array;

struct _int_array
{
  int *p;
  int len;
  int ref_count;
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
  int len;
  int ref_count;
};

pointer_array *_pointer_array_new         (void);
pointer_array *_pointer_array_append_data (pointer_array *a, void *p);
pointer_array *_pointer_array_ref         (pointer_array *a);
int            _pointer_array_find_data   (pointer_array *a, void *p);

void           _pointer_array_unref       (pointer_array *a);

#ifdef	__cplusplus
}
#endif

#endif /* __ARRAY_H__ */
