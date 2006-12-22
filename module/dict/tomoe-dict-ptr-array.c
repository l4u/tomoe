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

#include <string.h>

#include "tomoe-dict-ptr-array.h"

#include <tomoe-candidate.h>
#include <glib-utils.h>

typedef struct _TomoeDictSearchContext {
    TomoeQuery *query;
    GList *results;
} TomoeDictSearchContext;

static gint
char_compare_func (gconstpointer a, gconstpointer b)
{
    TomoeChar *ca = *(TomoeChar **) a;
    TomoeChar *cb = *(TomoeChar **) b;
    return tomoe_char_compare (ca, cb);
}

void
_tomoe_dict_ptr_array_sort (GPtrArray *chars)
{
    g_ptr_array_sort (chars, char_compare_func);
}

gboolean
_tomoe_dict_ptr_array_register_char (GPtrArray *chars, TomoeChar *chr)
{
    g_ptr_array_add (chars, g_object_ref (G_OBJECT (chr)));
    _tomoe_dict_ptr_array_sort (chars);

    return TRUE;
}

gboolean
_tomoe_dict_ptr_array_unregister_char (GPtrArray *chars, const gchar *utf8)
{
    TomoeChar *removed = NULL;
    gint i, len, index = -1;

    g_return_val_if_fail (utf8 && *utf8 != '\0', FALSE);

    len = chars->len;
    for (i = 0; i < len; i++) {
        TomoeChar *chr = g_ptr_array_index (chars, i);
        if (g_str_equal (tomoe_char_get_utf8(chr), utf8)) {
            index = i;
            removed = chr;
            break;
        }
    }

    if (index >= 0) {
        g_ptr_array_remove_index (chars, index);
        g_object_unref (removed);
        return TRUE;
    } else {
        return FALSE;
    }
}

TomoeChar *
_tomoe_dict_ptr_array_get_char (GPtrArray *chars, const gchar *utf8)
{
    gint i, len;

    g_return_val_if_fail (utf8 && *utf8 != '\0', NULL);

    len = chars->len;
    for (i = 0; i < len; i++) {
        TomoeChar *chr = g_ptr_array_index (chars, i);
        if (g_str_equal (tomoe_char_get_utf8(chr), utf8)) {
            return g_object_ref (chr);
        }
    }

    return NULL;
}

static gboolean
does_match_char_with_utf8 (TomoeChar *chr, const gchar *utf8)
{
    if (!utf8)
        return TRUE;

    return g_utf8_collate (tomoe_char_get_utf8 (chr), utf8) == 0;
}

static gboolean
does_match_char_with_variant (TomoeChar *chr, const gchar *variant)
{
    const gchar *chr_variant;

    if (!variant)
        return TRUE;

    chr_variant = tomoe_char_get_variant (chr);
    if (!chr_variant)
        return FALSE;

    return g_utf8_collate (chr_variant, variant) == 0;
}

static gboolean
does_match_char_with_n_strokes (TomoeChar *chr, gint min, gint max)
{
    TomoeWriting *writing;
    gint n_strokes;

    if (min < 0 && max < 0)
        return TRUE;

    n_strokes = tomoe_char_get_n_strokes (chr);
    if (n_strokes < 0) {
        writing = tomoe_char_get_writing (chr);
        if (!writing)
            return FALSE;
        n_strokes = tomoe_writing_get_n_strokes (writing);
    }

    return ((min < 0 || min <= n_strokes) &&
            (max < 0 || max >= n_strokes));
}

static gint
compare_reading (gconstpointer a, gconstpointer b)
{
    TomoeReading *reading, *searched_reading;

    reading = TOMOE_READING(a);
    searched_reading = TOMOE_READING(b);
    return strcmp(tomoe_reading_get_reading(reading),
                  tomoe_reading_get_reading(searched_reading));
}

static gboolean
does_match_char_with_reading (TomoeChar *chr, TomoeReading *reading)
{
    if (!reading)
        return TRUE;

    if (g_list_find_custom ((GList *)tomoe_char_get_readings (chr),
                            reading, compare_reading))
        return TRUE;
    else
        return FALSE;
}

static gboolean
does_match_char_with_readings (TomoeChar *chr, const GList *readings)
{
    GList *node;

    for (node = (GList *)readings; node; node = g_list_next (node)) {
        TomoeReading *reading = node->data;
        if (!does_match_char_with_reading (chr, reading))
            return FALSE;
    }

    return TRUE;
}

static gboolean
does_match_char_with_radical (TomoeChar *chr, const gchar *radical)
{
    if (!radical)
        return TRUE;

    if (g_list_find_custom ((GList *)tomoe_char_get_radicals (chr),
                            radical, (GCompareFunc)g_utf8_collate))
        return TRUE;
    else
        return FALSE;
}

static gboolean
does_match_char_with_radicals (TomoeChar *chr, const GList *radicals)
{
    GList *node;

    for (node = (GList *)radicals; node; node = g_list_next (node)) {
        const gchar *radical = node->data;
        if (!does_match_char_with_radical (chr, radical))
            return FALSE;
    }

    return TRUE;
}

static void
collect_chars_by_query (gpointer data, gpointer user_data)
{
    TomoeChar *chr = data;
    TomoeDictSearchContext *context = user_data;
    TomoeQuery *q;

    q = context->query;
    if (does_match_char_with_utf8 (chr, tomoe_query_get_utf8 (q)) &&
        does_match_char_with_variant (chr, tomoe_query_get_variant (q)) &&
        does_match_char_with_n_strokes (chr,
                                        tomoe_query_get_min_n_strokes (q),
                                        tomoe_query_get_max_n_strokes (q)) &&
        does_match_char_with_readings (chr, tomoe_query_get_readings (q)) &&
        does_match_char_with_radicals (chr, tomoe_query_get_radicals (q))) {
        context->results = g_list_prepend (context->results,
                                           tomoe_candidate_new (chr));
    }
}

static void
collect_all_chars (gpointer data, gpointer user_data)
{
    TomoeChar *chr = data;
    TomoeDictSearchContext *context = user_data;

    context->results = g_list_prepend (context->results,
                                       tomoe_candidate_new (chr));
}

GList *
_tomoe_dict_ptr_array_search (GPtrArray *chars, TomoeQuery *query)
{
    TomoeDictSearchContext search_context;

    search_context.query = g_object_ref (query);
    search_context.results = NULL;

    if (tomoe_query_is_empty (query)) {
        g_ptr_array_foreach_reverse (chars, collect_all_chars,
                                     &search_context);
    } else {
        g_ptr_array_foreach_reverse (chars, collect_chars_by_query,
                                     &search_context);
    }
    g_object_unref (search_context.query);

    return search_context.results;
}

gchar *
_tomoe_dict_ptr_array_get_available_private_utf8 (GPtrArray *chars)
{
    gint i, len, result_len;
    gchar *result;
    gunichar result_ucs = TOMOE_CHAR_PRIVATE_USE_AREA_START;

    len = chars->len;
    for (i = 0; i < len; i++) {
        TomoeChar *chr;
        gunichar ucs;

        chr = chars->pdata[i];
        ucs = g_utf8_get_char (tomoe_char_get_utf8 (chr));
        if (ucs >= TOMOE_CHAR_PRIVATE_USE_AREA_START) {
            if (ucs >= TOMOE_CHAR_PRIVATE_USE_AREA_END) {
                return NULL;
            } else {
                result_ucs = ucs + 1;
            }
        }
    }

    result_len = g_unichar_to_utf8 (result_ucs, NULL);
    result = g_new (gchar, result_len + 1);
    g_unichar_to_utf8 (result_ucs, result);
    result[result_len] = '\0';
    return result;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
