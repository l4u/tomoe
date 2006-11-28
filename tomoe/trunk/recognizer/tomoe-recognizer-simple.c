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

#include <stdlib.h>

#include "tomoe-recognizer-impl.h"
#include "tomoe-recognizer-simple-logic.h"

typedef struct _TomoeRecognizerSimple TomoeRecognizerSimple;

struct _TomoeRecognizerSimple
{
    gint ref;
};


gpointer
TOMOE_RECOGNIZER_IMPL_NEW (void)
{
    TomoeRecognizerSimple *recognizer;
    recognizer = calloc (1, sizeof (TomoeRecognizerSimple));
    if (!recognizer) return NULL;

    recognizer->ref = 1;
    return recognizer;
}

void
TOMOE_RECOGNIZER_IMPL_FREE (gpointer context)
{
    TomoeRecognizerSimple *recognizer = context;
    if (!recognizer) return;
    free (recognizer);
}

GList *
TOMOE_RECOGNIZER_IMPL_SEARCH (gpointer context, TomoeDict *dict,
                              TomoeWriting *input)
{
    return _tomoe_recognizer_simple_get_candidates (context, dict, input);
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
