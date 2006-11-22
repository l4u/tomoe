/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
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

#include <stdlib.h>
#include "tomoe-writing.h"

#define TOMOE_WRITING_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_WRITING, TomoeWritingPrivate))

typedef struct _TomoeWritingPrivate TomoeWritingPrivate;
typedef struct _TomoeStroke       TomoeStroke;

struct _TomoeWritingPrivate
{
    GList *stroke_first;
    GList *stroke_last;
};

struct _TomoeStroke
{
    GList *point_first;
    GList *point_last;
};

G_DEFINE_TYPE (TomoeWriting, tomoe_writing, G_TYPE_OBJECT)

static void tomoe_writing_dispose (GObject *object);
static TomoePoint  *_point_new  (gint x, gint y);
static TomoeStroke *_stroke_new (gint x, gint y);

static void
tomoe_writing_class_init (TomoeWritingClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = tomoe_writing_dispose;

    g_type_class_add_private (gobject_class, sizeof (TomoeWritingPrivate));
}

static void
tomoe_writing_init (TomoeWriting *writing)
{
    TomoeWritingPrivate *priv = TOMOE_WRITING_GET_PRIVATE(writing);
    priv->stroke_first = NULL;
    priv->stroke_last  = NULL;
}

static void
tomoe_writing_dispose (GObject *object)
{
    TomoeWriting *writing = TOMOE_WRITING (object);

    tomoe_writing_clear (writing);

    G_OBJECT_CLASS (tomoe_writing_parent_class)->dispose (object);
}

TomoeWriting *
tomoe_writing_new (void)
{
    TomoeWriting *writing = g_object_new(TOMOE_TYPE_WRITING, NULL);
    return writing;
}

void
tomoe_writing_move_to (TomoeWriting *writing, gint x, gint y)
{
    TomoeWritingPrivate *priv;

    g_return_if_fail (TOMOE_IS_WRITING (writing));

    priv = TOMOE_WRITING_GET_PRIVATE(writing);

    priv->stroke_last = g_list_append (priv->stroke_last, _stroke_new (x, y));
    if (!priv->stroke_first)
        priv->stroke_first = priv->stroke_last;
    priv->stroke_last = g_list_last (priv->stroke_last);
}

void
tomoe_writing_line_to (TomoeWriting *writing, gint x, gint y)
{
    TomoeWritingPrivate *priv;
    TomoeStroke *s;

    g_return_if_fail (TOMOE_IS_WRITING (writing));

    priv = TOMOE_WRITING_GET_PRIVATE(writing);
    g_return_if_fail (priv->stroke_last);

    s = priv->stroke_last->data;
    g_return_if_fail (s);

    s->point_last = g_list_append (s->point_last, _point_new (x, y));
    s->point_last = g_list_last (s->point_last);
}

void
tomoe_writing_clear (TomoeWriting *writing)
{
    TomoeWritingPrivate *priv;
    GList *node;

    priv = TOMOE_WRITING_GET_PRIVATE(writing);
    g_return_if_fail (priv);

    for (node = priv->stroke_first; node; node = g_list_next (node)) {
        TomoeStroke *s;

        if (!node->data) continue;

        s = node->data;
        g_list_foreach (s->point_first, (GFunc) g_free, NULL);
        g_list_free (s->point_first);
        g_free (s);
    }
    g_list_free (priv->stroke_first);

    priv->stroke_first = NULL;
    priv->stroke_last  = NULL;
}

guint
tomoe_writing_get_number_of_strokes (TomoeWriting *writing)
{
    TomoeWritingPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_WRITING (writing), 0);

    priv = TOMOE_WRITING_GET_PRIVATE(writing);
    g_return_val_if_fail (priv, 0);

    return g_list_length (priv->stroke_first);
}

guint
tomoe_writing_get_number_of_points (TomoeWriting *writing, guint stroke)
{
    TomoeWritingPrivate *priv;
    TomoeStroke *s;

    g_return_val_if_fail (TOMOE_IS_WRITING (writing), 0);

    priv = TOMOE_WRITING_GET_PRIVATE(writing);
    g_return_val_if_fail (priv && priv->stroke_first, 0);

    s = g_list_nth_data (priv->stroke_first, stroke);
    g_return_val_if_fail (s, 0);

    return g_list_length (s->point_first);
}

gboolean
tomoe_writing_get_point (TomoeWriting *writing, guint stroke, guint point,
                       gint *x, gint *y)
{
    TomoeWritingPrivate *priv;
    TomoeStroke *s;
    TomoePoint *p;

    if (x) *x = 0;
    if (y) *y = 0;

    g_return_val_if_fail (TOMOE_IS_WRITING (writing), FALSE);

    priv = TOMOE_WRITING_GET_PRIVATE(writing);
    g_return_val_if_fail (priv && priv->stroke_first, FALSE);

    s = g_list_nth_data (priv->stroke_first, stroke);
    g_return_val_if_fail (s, FALSE);
    g_return_val_if_fail (s->point_first, FALSE);

    p = g_list_nth_data (s->point_first, point);
    g_return_val_if_fail (p, FALSE);

    if (x) *x = p->x;
    if (y) *y = p->y;

    return TRUE;
}

gboolean
tomoe_writing_get_last_point (TomoeWriting *writing, gint *x, gint *y)
{
    TomoeWritingPrivate *priv;
    TomoeStroke *s;
    TomoePoint *p;

    if (x) *x = 0;
    if (y) *y = 0;

    g_return_val_if_fail (TOMOE_IS_WRITING (writing), FALSE);

    priv = TOMOE_WRITING_GET_PRIVATE(writing);
    g_return_val_if_fail (priv && priv->stroke_last, FALSE);

    s = priv->stroke_last->data;
    g_return_val_if_fail (s && s->point_last && s->point_last->data, FALSE);

    p = s->point_last->data;

    if (x) *x = p->x;
    if (y) *y = p->y;

    return TRUE;
}

void
tomoe_writing_remove_last_stroke (TomoeWriting *writing)
{
    TomoeWritingPrivate *priv;
    TomoeStroke *s;

    g_return_if_fail (TOMOE_IS_WRITING (writing));

    priv = TOMOE_WRITING_GET_PRIVATE(writing);
    g_return_if_fail (priv);
    if (!priv->stroke_last) return;

    s = priv->stroke_last->data;
    g_return_if_fail (s);

    g_list_foreach (s->point_first, (GFunc) g_free, NULL);
    g_list_free (s->point_first);

    priv->stroke_last = g_list_remove (priv->stroke_last, s);
    priv->stroke_last = g_list_last (priv->stroke_last);
    if (!priv->stroke_last)
        priv->stroke_first = NULL;
    g_free (s);
}

const GList *
tomoe_writing_get_strokes (TomoeWriting  *writing)
{
    TomoeWritingPrivate *priv;

    g_return_val_if_fail (TOMOE_IS_WRITING (writing), NULL);

    priv = TOMOE_WRITING_GET_PRIVATE(writing);

    return priv->stroke_first;
}

static TomoePoint *
_point_new (gint x, gint y)
{
    TomoePoint *p = g_new (TomoePoint, 1);

    g_return_val_if_fail (p, NULL);

    p->x = x;
    p->y = y;

    return p;
}

static TomoeStroke *
_stroke_new (gint x, gint y)
{
    TomoeStroke *s = g_new (TomoeStroke, 1);

    g_return_val_if_fail (s, NULL);

    s->point_first = g_list_append (NULL, _point_new (x, y));
    s->point_last  = s->point_first;

    return s;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
