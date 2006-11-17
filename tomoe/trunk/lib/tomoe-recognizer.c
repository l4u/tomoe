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

#include <gmodule.h>
#include "tomoe-recognizer.h"
#include "tomoe-recognizer-impl.h"

#define RECOGNIZER_NEW_FUNC "tomoe_recognizer_impl_new"
#define RECOGNIZER_FREE_FUNC "tomoe_recognizer_impl_free"
#define RECOGNIZER_SEARCH_FUNC "tomoe_recognizer_impl_search"

typedef struct _TomoeRecognizerInfo TomoeRecognizerInfo;

struct _TomoeRecognizerInfo
{
    GModule *module;
    void *context;
};

struct _TomoeRecognizer
{
    int ref;
    TomoeRecognizerInfo *info;
};

static void
tomoe_recognizer_show_error (GModule *module)
{
    gchar *message;

    message = g_locale_to_utf8(g_module_error(), -1, NULL, NULL, NULL);

    if (module) {
        gchar *name;
        name = g_strdup(g_module_name(module));
        g_warning("%s: %s", name, message);
        g_free(name);
    } else {
        g_warning("%s", message);
    }

    g_free(message);
}

static gboolean
tomoe_recognizer_load_func (GModule *module, const gchar *func_name,
                            gpointer *symbol)
{
    if (g_module_symbol(module, func_name, symbol)) {
        return TRUE;
    } else {
        tomoe_recognizer_show_error(module);
        return FALSE;
    }
}

static void
tomoe_recognizer_close_module (GModule *module, void *recognizer)
{
    TomoeRecognizerFreeFunc free_func;
    TomoeRecognizerFreeFunc *free_func_p;
    gpointer *p;

    free_func_p = &free_func;
    p = (gpointer *)free_func_p;
    if (tomoe_recognizer_load_func(module, RECOGNIZER_FREE_FUNC, p))
        free_func(recognizer);
    else
        tomoe_recognizer_show_error(module);

    if (!g_module_close(module))
        tomoe_recognizer_show_error(module);
}

static void
tomoe_recognizer_info_free (TomoeRecognizerInfo *info)
{
    if (info->module)
    {
        tomoe_recognizer_close_module(info->module, info->context);
        info->module = NULL;
        info->context = NULL;
    }
}

static TomoeRecognizerInfo *
tomoe_recognizer_load(const char *name)
{
    gchar *mod_path;
    GModule *module;
    TomoeRecognizerInfo *info = NULL;

    mod_path = g_module_build_path(RECOGNIZERDIR, name);
    module = g_module_open(mod_path, G_MODULE_BIND_LAZY);

    if (module) {
        TomoeRecognizerNewFunc new_func;
        TomoeRecognizerNewFunc *new_func_p;
        gpointer *p;

        new_func_p = &new_func;
        p = (gpointer *)new_func_p;
        if (tomoe_recognizer_load_func(module, RECOGNIZER_NEW_FUNC, p)) {
            info = g_new0(TomoeRecognizerInfo, 1);
            info->context = new_func();
            info->module = module;
        } else {
            tomoe_recognizer_close_module(module, NULL);
        }
    } else {
        tomoe_recognizer_show_error(NULL);
    }

    g_free(mod_path);

    return info;
}

static TomoeRecognizerInfo *
tomoe_recognizer_find_recognizer(void)
{
    TomoeRecognizerInfo *info = NULL;
    GDir *dir;

    dir = g_dir_open(RECOGNIZERDIR, 0, NULL);
    if (dir) {
        const gchar *entry;

        while ((entry = g_dir_read_name(dir))) {
            info = tomoe_recognizer_load(entry);
            if (info) break;
        }

        g_dir_close(dir);
    }

    return info;
}

TomoeRecognizer *
tomoe_recognizer_new (void)
{
    TomoeRecognizer *recognizer;
    recognizer = calloc (1, sizeof (TomoeRecognizer));
    if (!recognizer) return NULL;

    recognizer->info = tomoe_recognizer_find_recognizer();
    if (!recognizer->info) {
        free(recognizer);
        return NULL;
    }

    recognizer->ref = 1;
    return recognizer;
}

void
tomoe_recognizer_free (TomoeRecognizer *recognizer)
{
    if (!recognizer) return;

    recognizer->ref--;
    if (recognizer->ref <= 0)
    {
        if (recognizer->info) {
            tomoe_recognizer_info_free(recognizer->info);
            recognizer->info = NULL;
        }
        free (recognizer);
    }
}

TomoeRecognizer *
tomoe_recognizer_add_ref (TomoeRecognizer *recognizer)
{
    if (!recognizer) return NULL;
    recognizer->ref++;
    return recognizer;
}

TomoeArray *
tomoe_recognizer_search (const TomoeRecognizer *recognizer,
                         TomoeDict *dict, TomoeGlyph *input)
{
    TomoeArray *result = NULL;
    TomoeRecognizerSearchFunc search_func;
    TomoeRecognizerSearchFunc *search_func_p;
    gpointer *p;

    search_func_p = &search_func;
    p = (gpointer *)search_func_p;
    if (tomoe_recognizer_load_func(recognizer->info->module,
                                   RECOGNIZER_SEARCH_FUNC, p)) {
        result = search_func(recognizer->info->context, dict, input);
    } else {
        tomoe_recognizer_show_error(recognizer->info->module);
    }

    return result;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
