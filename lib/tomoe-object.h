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

/**
 *  @file tomoe-object.h
 *  @brief Provides base class for all tomoe classes
 */

#ifndef __TOMOE_OBJECT_H__
#define __TOMOE_OBJECT_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "tomoe-data-types.h"

typedef struct _TomoeObject
{
} TomoeObject;

typedef struct _TomoeObjectClass
{
} TomoeObjectClass;

typedef void  (*tomoe_class_init_fn)      (TomoeObjectClass *cls);
typedef void  (*tomoe_inst_init_fn)       (TomoeObject      *obj);
typedef void  (*tomoe_inst_finalize_fn)   (TomoeObject      *obj);

typedef struct _tomoe_type_info
{
    char                  *name;
    size_t                 class_size;
    /* tomoe_class_init_fn    class_init; */
    tomoe_inst_init_fn     base_init;
    tomoe_inst_init_fn     base_finalize;
    tomoe_inst_init_fn     inst_init;
    tomoe_inst_finalize_fn inst_finalize;
    size_t                 inst_size;
} tomoe_type_info;

typedef int tomoe_class_type;

void             tomoe_class_type_register_init ();
void             tomoe_class_type_register_term ();

TomoeObject*     TomoeObject_new          (tomoe_class_type type);
TomoeObject*     TomoeObject_addRef       (TomoeObject     *obj);
void             TomoeObject_free         (TomoeObject     *obj);

tomoe_class_type tomoe_register_class_type (tomoe_type_info typeinfo);

#define TOMOE_CLASS_BEGIN(i, b) \
 typedef struct _##i i;\
 typedef struct _##i##Class { b##Class parent_class; } i##Class;\
 tomoe_class_type i##_getType (void);\
 i* i##_new (void); \
 struct _##i { b parent_instance;
#define TOMOE_CLASS_END };

#define TOMOE_CLASS_IMPL(i) i* i##_new (void)\
 { return (i*)TomoeObject_new (i##_getType ()); };\
 tomoe_class_type i##_getType (void)\
 { return 0; };
#define TOMOE_CLASS_PRIV_BEGIN(i) struct _##i##Priv {
#define TOMOE_CLASS_PRIV_END };


#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_OBJECT_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
