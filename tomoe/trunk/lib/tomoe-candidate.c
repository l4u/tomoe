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

TomoeCandidate*
tomoe_candidate_new (void)
{
    TomoeCandidate* cand;

    cand            = calloc (sizeof (TomoeCandidate), 1);
    cand->ref       = 1;
    cand->character = NULL;
    cand->score     = 0;

    return cand;
}

TomoeCandidate*
tomoe_candidate_add_ref (TomoeCandidate* t_cand)
{
    if (!t_cand) return NULL;
    t_cand->ref ++;
    return t_cand;
}

void
tomoe_candidate_free (TomoeCandidate* t_cand)
{
    if (!t_cand) return;
    t_cand->ref --;
    if (t_cand->ref <= 0) {
        g_object_unref (G_OBJECT (t_cand->character));
        free (t_cand);
    }
}

int
tomoe_candidate_compare (const TomoeCandidate *a, const TomoeCandidate *b)
{
    int score_a = a->score;
    int score_b = b->score;

    return score_a > score_b ? 1
        : score_a < score_b ? - 1
        : 0;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
