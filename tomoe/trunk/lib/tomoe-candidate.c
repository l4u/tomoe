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

#include "tomoe-candidate.h"

#define TOMOE_CANDIDATE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_CANDIDATE, TomoeCandidatePrivate))

typedef struct _TomoeCandidatePrivate	TomoeCandidatePrivate;
struct _TomoeCandidatePrivate
{
    TomoeChar    *character;
    gint          score;
};

G_DEFINE_TYPE (TomoeCandidate, tomoe_candidate, G_TYPE_OBJECT)

static void tomoe_candidate_dispose (GObject *object);

static void
tomoe_candidate_class_init (TomoeCandidateClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = tomoe_candidate_dispose;

    g_type_class_add_private (gobject_class, sizeof (TomoeCandidatePrivate));
}

static void
tomoe_candidate_init (TomoeCandidate *cand)
{
    TomoeCandidatePrivate *priv = TOMOE_CANDIDATE_GET_PRIVATE (cand);

    priv->character = NULL;
    priv->score     = 0;
}

static void
tomoe_candidate_dispose (GObject *object)
{
    TomoeCandidatePrivate *priv = TOMOE_CANDIDATE_GET_PRIVATE (object);

    if (priv->character)
        g_object_unref (G_OBJECT (priv->character));
    priv->character = NULL;

    G_OBJECT_CLASS (tomoe_candidate_parent_class)->dispose (object);
}

TomoeCandidate*
tomoe_candidate_new (TomoeChar *t_char)
{
    TomoeCandidate *cand;
    TomoeCandidatePrivate *priv;

    cand = g_object_new(TOMOE_TYPE_CANDIDATE,
                        /* "tomoe-char", t_char, */
                        NULL);

    /* FIXME: use property */
    if (t_char) {
        priv = TOMOE_CANDIDATE_GET_PRIVATE (cand);
        priv->character = g_object_ref (G_OBJECT (t_char));
    }

    return cand;
}

TomoeChar *
tomoe_candidate_get_char (TomoeCandidate *cand)
{
    TomoeCandidatePrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CANDIDATE (cand), NULL);

    priv = TOMOE_CANDIDATE_GET_PRIVATE (cand);
    g_return_val_if_fail (priv, NULL);

    return priv->character;
}

void
tomoe_candidate_set_score (TomoeCandidate *cand, gint score)
{
    TomoeCandidatePrivate *priv;

    g_return_if_fail (TOMOE_IS_CANDIDATE (cand));

    priv = TOMOE_CANDIDATE_GET_PRIVATE (cand);
    g_return_if_fail (priv);

    priv->score = score;
}

gint
tomoe_candidate_get_score (TomoeCandidate *cand)
{
    TomoeCandidatePrivate *priv;

    g_return_val_if_fail (TOMOE_IS_CANDIDATE (cand), 0);

    priv = TOMOE_CANDIDATE_GET_PRIVATE (cand);
    g_return_val_if_fail (priv, 0);

    return priv->score;
}

gint
tomoe_candidate_compare (const TomoeCandidate *a, const TomoeCandidate *b)
{
    TomoeCandidatePrivate *priv_a, *priv_b;

    if (!TOMOE_IS_CANDIDATE (a) || !TOMOE_IS_CANDIDATE (b)) return 0;

    priv_a = TOMOE_CANDIDATE_GET_PRIVATE (a);
    priv_b = TOMOE_CANDIDATE_GET_PRIVATE (b);
    if (!priv_a || !priv_b) return 0;

    return priv_a->score > priv_b->score ? 1
         : priv_a->score < priv_b->score ? - 1
         : 0;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
