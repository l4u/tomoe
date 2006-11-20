/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "tomoe-dict.h"
#include "tomoe-shelf.h"
#include "glib-utils.h"

#define TOMOE_SHELF_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_SHELF, TomoeShelfPrivate))

typedef struct _TomoeShelfPrivate	TomoeShelfPrivate;
struct _TomoeShelfPrivate
{
    GPtrArray  *dicts;
};

G_DEFINE_TYPE (TomoeShelf, tomoe_shelf, G_TYPE_OBJECT)

static void tomoe_shelf_dispose      (GObject *object);

static void
tomoe_shelf_class_init (TomoeShelfClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose  = tomoe_shelf_dispose;

    g_type_class_add_private (gobject_class, sizeof (TomoeShelfPrivate));
}

static void
tomoe_shelf_init (TomoeShelf *shelf)
{
    TomoeShelfPrivate *priv = TOMOE_SHELF_GET_PRIVATE (shelf);

    priv->dicts      = g_ptr_array_new ();
}

TomoeShelf*
tomoe_shelf_new(void)
{
    TomoeShelf *shelf;

    shelf = g_object_new(TOMOE_TYPE_SHELF, NULL);

    return shelf;
}

static void
tomoe_shelf_dispose (GObject *object)
{
    TomoeShelfPrivate *priv = TOMOE_SHELF_GET_PRIVATE (object);

    if (priv->dicts) {
        TOMOE_PTR_ARRAY_FREE_ALL (priv->dicts, g_object_unref);
    }

    priv->dicts      = NULL;

    G_OBJECT_CLASS (tomoe_shelf_parent_class)->dispose (object);
}

void
tomoe_shelf_add_dict (TomoeShelf *shelf, TomoeDict *dict)
{
    TomoeShelfPrivate *priv;
    g_return_if_fail (shelf || dict);
    priv = TOMOE_SHELF_GET_PRIVATE (shelf);
    g_ptr_array_add (priv->dicts, g_object_ref (dict));
}

void
tomoe_shelf_load_dict (TomoeShelf *shelf, const char *filename,
                       gboolean editable)
{
    TomoeDict* dict;

    g_return_if_fail(shelf);
    g_return_if_fail(filename);

    dict = tomoe_dict_new (filename, editable);
    if (dict) {
        tomoe_shelf_add_dict (shelf, dict);
        g_object_unref (dict);
    }
}

const GPtrArray*
tomoe_shelf_get_dict_list (TomoeShelf* shelf)
{
    TomoeShelfPrivate *priv;
    g_return_val_if_fail (shelf, NULL);

    priv = TOMOE_SHELF_GET_PRIVATE (shelf);
    return priv->dicts;
}

void
tomoe_shelf_save (TomoeShelf *shelf)
{
    TomoeShelfPrivate *priv;
    guint i;

    g_return_if_fail (shelf);
    priv = TOMOE_SHELF_GET_PRIVATE (shelf);

    for (i = 0; i < priv->dicts->len; i++) {
        TomoeDict *dict = (TomoeDict*) g_ptr_array_index (priv->dicts, i);
        if (tomoe_dict_is_modified (dict))
            tomoe_dict_save (dict);
    }
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
