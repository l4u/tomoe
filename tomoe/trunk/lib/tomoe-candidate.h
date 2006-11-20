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

/** @file tomoe-candidate.h
 *  @brief 
 */

#ifndef __TOMOE_CANDIDATE_H__
#define __TOMOE_CANDIDATE_H__

#include <glib-object.h>
#include <tomoe-char.h>

G_BEGIN_DECLS

typedef struct _TomoeCandidate TomoeCandidate;

struct _TomoeCandidate
{
    int           ref;
    TomoeChar    *character;
    int           score;
};

TomoeCandidate *tomoe_candidate_new             (void);

TomoeCandidate *tomoe_candidate_add_ref         (TomoeCandidate  *t_cand);

void            tomoe_candidate_free            (TomoeCandidate  *t_cand);

/**
 * @brief Compare two TomoeCandidate.
 * @param a - 1st TomoeCandidate object to compare.
 * @param b - 2nd TomoeCandidate object to compare.
 * @return -1 a < b, 0 a= b, 1 a > b
 */
int             tomoe_candidate_compare         (const TomoeCandidate *a,
                                                 const TomoeCandidate *b);

G_END_DECLS

#endif /* __TOMOE_CANDIDATE_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
