/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tomoe-dict.h"
#include "tomoe-recognizer.h"
#include "tomoe-context.h"
#include "tomoe-array.h"

struct _TomoeContext
{
    int         ref;
    GPtrArray  *dicts;
    TomoeRecognizer *recognizer;
};


TomoeContext*
tomoe_context_new(void)
{
    TomoeContext* p;
    p        = calloc (1, sizeof(TomoeContext));
    p->ref   = 1;
    p->dicts = g_ptr_array_new ();
    p->recognizer = tomoe_recognizer_new();
    return p;
}

TomoeContext*
tomoe_context_add_ref(TomoeContext* ctx)
{
    if (!ctx) return NULL;
    ctx->ref++;
    return ctx;
}
static void
_dict_free (gpointer data, gpointer user_data)
{
    TomoeDict *dict = (TomoeDict *) data;

    tomoe_dict_free (dict);
}

void
tomoe_context_free(TomoeContext* ctx)
{
    if (!ctx) return;
    ctx->ref--;
    if (ctx->ref <= 0) {
        g_ptr_array_foreach (ctx->dicts, _dict_free, NULL);
        g_ptr_array_free (ctx->dicts, FALSE);
        tomoe_recognizer_free (ctx->recognizer);
        free (ctx);
    }
}

void
tomoe_context_add_dict (TomoeContext* ctx, TomoeDict* dict)
{
    if (!ctx || !dict) return;
    g_ptr_array_add (ctx->dicts, dict);
}

void
tomoe_context_load_dict (TomoeContext* ctx, const char *filename, int editable)
{
    TomoeDict* dict;

    if (!ctx) return;
    if (!filename) return;

    fprintf (stdout, "load dictionary '%s' editable: %s...",
             filename, editable ? "yes" : "no");
    fflush (stdout);
    dict = tomoe_dict_new (filename, editable);
    if (dict)
        g_ptr_array_add (ctx->dicts, dict);
    printf (" ok\n");
}

void
tomoe_context_load_dict_list (TomoeContext* ctx, const GPtrArray* list)
{
    int i;
    for (i = 0; i < list->len; i++)
    {
        TomoeDictCfg* p = g_ptr_array_index (list, i);
        if (p->dontLoad) continue;

        if (p->user)
            tomoe_context_load_dict (ctx, p->filename, p->writeAccess);
        else
        {
            char* file = calloc (strlen (p->filename) +
                                 strlen (TOMOEDATADIR) + 2,
                                 sizeof (char));
            strcpy (file, TOMOEDATADIR);
            strcat (file, "/");
            strcat (file, p->filename);
            tomoe_context_load_dict (ctx, file, p->writeAccess);
        }
    }
}

const GPtrArray*
tomoe_context_get_dict_list (TomoeContext* ctx)
{
    if (!ctx) return NULL;
    return ctx->dicts;
}

void
tomoe_context_save (TomoeContext *ctx)
{
    guint i;

    if (!ctx) return;

    for (i = 0; i < ctx->dicts->len; i++)
    {
        TomoeDict *dict = (TomoeDict*) g_ptr_array_index (ctx->dicts, i);
        if (tomoe_dict_is_modified (dict))
            tomoe_dict_save (dict);
    }
}

TomoeArray*
tomoe_context_search_by_strokes (TomoeContext* ctx, TomoeGlyph* input)
{
    guint i, num;
    TomoeArray* tmp;
    TomoeArray* matched;
    TomoeDict* dict;

    if (!ctx) return tomoe_array_new (NULL, NULL, NULL);
    num = ctx->dicts->len;
    if (num == 0) return tomoe_array_new (NULL, NULL, NULL);

    dict = (TomoeDict*)g_ptr_array_index (ctx->dicts, 0);
    tmp = tomoe_recognizer_search(ctx->recognizer, dict, input);
    matched = tomoe_array_clone_empty (tmp);
    for (i = 0; i < num; i++) {
        TomoeArray* tmp;
        dict = (TomoeDict*)g_ptr_array_index (ctx->dicts, i);
        tmp = tomoe_recognizer_search(ctx->recognizer, dict, input);
        tomoe_array_merge (matched, tmp);
        tomoe_array_free (tmp);
    }
    tomoe_array_sort (matched);

    return matched;
}

#warning FIXME!
static void
ptr_array_merge_func (gpointer data, gpointer user_data)
{
    GPtrArray *p = (GPtrArray *) user_data;
    g_ptr_array_add (p, data);
}

GPtrArray*
tomoe_context_search_by_reading (TomoeContext* ctx, const char* input)
{
    guint i, num;
    GPtrArray *reading = g_ptr_array_new ();

    if (!ctx) return reading;
    num = ctx->dicts->len;
    if (num == 0) return reading;

    for (i = 0; i < num; i++) {
        GPtrArray *tmp;
    	TomoeDict *dict;
        dict = (TomoeDict*) g_ptr_array_index (ctx->dicts, i);
        tmp = tomoe_dict_search_by_reading (dict, input);
	if (tmp) {
            g_ptr_array_foreach (tmp, ptr_array_merge_func, reading);
            g_ptr_array_free (tmp, FALSE);
	}
    }
    /*tomoe_array_sort (reading);*/

    return reading;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
