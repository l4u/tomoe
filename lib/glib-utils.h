/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
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


#ifndef __GLIB_UTILS_H__
#define __GLIB_UTILS_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib/garray.h>

#define TOMOE_PTR_ARRAY_FREE_ALL(p_array, free_func) \
{\
    g_ptr_array_foreach (p_array, (GFunc) (free_func), NULL);\
    g_ptr_array_free (p_array, TRUE);\
}

#ifdef	__cplusplus
}
#endif

#endif /* __GLIB_UTILS_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
