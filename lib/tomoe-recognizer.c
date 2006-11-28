
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

#include <stdlib.h>

#include "tomoe-recognizer.h"

G_DEFINE_ABSTRACT_TYPE (TomoeRecognizer, tomoe_recognizer, G_TYPE_OBJECT)

static void
tomoe_recognizer_class_init (TomoeRecognizerClass *klass)
{
    klass->search = NULL;
}

static void
tomoe_recognizer_init (TomoeRecognizer *recognizer)
{
}

GList *
tomoe_recognizer_search (TomoeRecognizer *recognizer,
                         TomoeDict *dict, TomoeWriting *input)
{
    TomoeRecognizerClass *klass;

    g_return_val_if_fail (TOMOE_IS_RECOGNIZER (recognizer), NULL);

    klass = TOMOE_RECOGNIZER_GET_CLASS (recognizer);
    if (klass->search)
        return klass->search (recognizer, dict, input);
    else
        return NULL;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
