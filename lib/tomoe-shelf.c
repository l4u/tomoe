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
    GHashTable *dicts;
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

    priv->dicts = g_hash_table_new_full(g_str_hash, g_str_equal,
                                        g_free, g_object_unref);
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
        g_hash_table_destroy(priv->dicts);
    }

    priv->dicts = NULL;

    G_OBJECT_CLASS (tomoe_shelf_parent_class)->dispose (object);
}

void
tomoe_shelf_add_dict (TomoeShelf *shelf, TomoeDict *dict)
{
    TomoeShelfPrivate *priv;
    g_return_if_fail (shelf);
    g_return_if_fail (dict);
    g_return_if_fail (tomoe_dict_get_name(dict));

    priv = TOMOE_SHELF_GET_PRIVATE (shelf);
    g_hash_table_insert (priv->dicts,
                         g_strdup (tomoe_dict_get_name (dict)),
                         g_object_ref (dict));
}

TomoeDict *
tomoe_shelf_get_dict (TomoeShelf *shelf, const gchar *name)
{
    TomoeShelfPrivate *priv;

    g_return_val_if_fail(shelf, NULL);
    g_return_val_if_fail(name, NULL);

    priv = TOMOE_SHELF_GET_PRIVATE (shelf);
    return g_hash_table_lookup(priv->dicts, name);
}

static void
tomoe_shelf_collect_dict_name (gpointer key, gpointer value, gpointer user_data)
{
    gchar *name = key;
    GList **names = user_data;
    
    *names = g_list_prepend(*names, g_strdup(name));
}

GList *
tomoe_shelf_get_dict_names (TomoeShelf* shelf)
{
    TomoeShelfPrivate *priv;
    GList *names = NULL;
    g_return_val_if_fail (shelf, NULL);

    priv = TOMOE_SHELF_GET_PRIVATE (shelf);
    g_hash_table_foreach(priv->dicts, tomoe_shelf_collect_dict_name, &names);
    return names;
}

static void
tomoe_shelf_save_dict (gpointer key, gpointer value, gpointer user_data)
{
    TomoeDict *dict = value;
    if (tomoe_dict_is_modified(dict))
        tomoe_dict_save(dict);
}

void
tomoe_shelf_save (TomoeShelf *shelf)
{
    TomoeShelfPrivate *priv;

    g_return_if_fail (shelf);
    priv = TOMOE_SHELF_GET_PRIVATE (shelf);

    g_hash_table_foreach(priv->dicts, tomoe_shelf_save_dict, NULL);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
