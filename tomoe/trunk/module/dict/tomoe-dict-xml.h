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

#ifndef __TOMOE_DICT_XML_H__
#define __TOMOE_DICT_XML_H__

#include <glib-object.h>

G_BEGIN_DECLS

#include "tomoe-char.h"
#include "tomoe-query.h"

#define TOMOE_TYPE_DICT_XML            (tomoe_dict_xml_get_type ())
#define TOMOE_DICT_XML(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_DICT_XML, TomoeDictXML))
#define TOMOE_DICT_XML_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_DICT_XML, TomoeDictXMLClass))
#define TOMOE_IS_DICT_XML(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_DICT_XML))
#define TOMOE_IS_DICT_XML_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_DICT_XML))
#define TOMOE_DICT_XML_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_DICT_XML, TomoeDictXMLClass))

typedef struct _TomoeDictXML TomoeDictXML;
typedef struct _TomoeDictXMLClass TomoeDictXMLClass;
struct _TomoeDictXML
{
    TomoeDict            object;
};

struct _TomoeDictXMLClass
{
    TomoeDictClass parent_class;
};

GType           tomoe_dict_xml_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __TOMOE_DICT_XML_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
