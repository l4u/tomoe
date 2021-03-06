/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Kouhei Sutou <kou@cozmixng.org>
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

#ifndef __TOMOE_UNIHAN_H__
#define __TOMOE_UNIHAN_H__

#include <glib.h>
#include <tomoe-char.h>
#include <tomoe-query.h>

G_BEGIN_DECLS

TomoeChar *_tomoe_unihan_get_char (const gchar *utf8);
GList     *_tomoe_unihan_search   (TomoeQuery  *query);

G_END_DECLS

#endif /*  __TOMOE_UNIHAN_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
