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

#include <gmodule.h>
#include "tomoe-recognizer.h"
#include "tomoe-recognizer-impl.h"

#define RECOGNIZER_NEW_FUNC "tomoe_recognizer_impl_new"
#define RECOGNIZER_FREE_FUNC "tomoe_recognizer_impl_free"
#define RECOGNIZER_SEARCH_FUNC "tomoe_recognizer_impl_search"

#define TOMOE_RECOGNIZER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_RECOGNIZER, TomoeRecognizerPrivate))

typedef struct _TomoeRecognizerPrivate TomoeRecognizerPrivate;
struct _TomoeRecognizerPrivate
{
    GModule *module;
    void *context;
};

G_DEFINE_TYPE (TomoeRecognizer, tomoe_recognizer, G_TYPE_OBJECT)

static void tomoe_recognizer_dispose     (GObject *object);

static void
tomoe_recognizer_class_init (TomoeRecognizerClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = tomoe_recognizer_dispose;

  g_type_class_add_private (gobject_class, sizeof (TomoeRecognizerPrivate));
}

static void
tomoe_recognizer_init (TomoeRecognizer *recognizer)
{
    TomoeRecognizerPrivate *priv = TOMOE_RECOGNIZER_GET_PRIVATE (recognizer);

    priv->module = NULL;
    priv->context = NULL;
}

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
tomoe_recognizer_close_module (GModule *module, void *context)
{
    TomoeRecognizerFreeFunc free_func;
    TomoeRecognizerFreeFunc *free_func_p;
    gpointer *p;

    free_func_p = &free_func;
    p = (gpointer *)free_func_p;
    if (tomoe_recognizer_load_func(module, RECOGNIZER_FREE_FUNC, p))
        free_func(context);
    else
        tomoe_recognizer_show_error(module);

    if (!g_module_close(module))
        tomoe_recognizer_show_error(module);
}

static gboolean
tomoe_recognizer_load(TomoeRecognizer *recognizer, const char *name)
{
    gboolean success = FALSE;
    gchar *mod_path;
    GModule *module;

    mod_path = g_module_build_path(RECOGNIZERDIR, name);
    module = g_module_open(mod_path, G_MODULE_BIND_LAZY);

    if (module) {
        TomoeRecognizerNewFunc new_func;
        TomoeRecognizerNewFunc *new_func_p;
        gpointer *p;

        new_func_p = &new_func;
        p = (gpointer *)new_func_p;
        if (tomoe_recognizer_load_func(module, RECOGNIZER_NEW_FUNC, p)) {
            TomoeRecognizerPrivate *priv;
            priv = TOMOE_RECOGNIZER_GET_PRIVATE (recognizer);
            priv->context = new_func();
            priv->module = module;
            success = TRUE;
        } else {
            tomoe_recognizer_close_module(module, NULL);
        }
    } else {
        tomoe_recognizer_show_error(NULL);
    }

    g_free(mod_path);

    return success;
}

static void
tomoe_recognizer_find_recognizer(TomoeRecognizer *recognizer)
{
    GDir *dir;

    dir = g_dir_open(RECOGNIZERDIR, 0, NULL);
    if (dir) {
        const gchar *entry;

        while ((entry = g_dir_read_name(dir))) {
            if (tomoe_recognizer_load(recognizer, entry)) break;
        }

        g_dir_close(dir);
    }
}

TomoeRecognizer *
tomoe_recognizer_new (void)
{
    TomoeRecognizer *recognizer;

    recognizer = g_object_new(TOMOE_TYPE_RECOGNIZER, NULL);
    tomoe_recognizer_find_recognizer(recognizer);

    return recognizer;
}

static void
tomoe_recognizer_dispose (GObject *object)
{
    TomoeRecognizer *recognizer;
    recognizer = TOMOE_RECOGNIZER (object);
    TomoeRecognizerPrivate *priv;

    priv = TOMOE_RECOGNIZER_GET_PRIVATE (recognizer);
    if (priv->module) {
        tomoe_recognizer_close_module(priv->module, priv->context);
    }

    G_OBJECT_CLASS (tomoe_recognizer_parent_class)->dispose (object);
}

GList *
tomoe_recognizer_search (const TomoeRecognizer *recognizer,
                         TomoeDict *dict, TomoeWriting *input)
{
    GList *result = NULL;
    TomoeRecognizerSearchFunc search_func;
    TomoeRecognizerSearchFunc *search_func_p;
    TomoeRecognizerPrivate *priv;
    gpointer *p;

    search_func_p = &search_func;
    p = (gpointer *)search_func_p;
    priv = TOMOE_RECOGNIZER_GET_PRIVATE (recognizer);
    if (tomoe_recognizer_load_func(priv->module,
                                   RECOGNIZER_SEARCH_FUNC, p)) {
        result = search_func(priv->context, dict, input);
    } else {
        tomoe_recognizer_show_error(priv->module);
    }

    return result;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
