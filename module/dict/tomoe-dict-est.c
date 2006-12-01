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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gmodule.h>

#include <estraier.h>

#include <tomoe-module-impl.h>
#include <tomoe-dict.h>
#include <tomoe-candidate.h>
#include <tomoe-xml-parser.h>
#include <glib-utils.h>

#define TOMOE_TYPE_DICT_EST            tomoe_type_dict_est
#define TOMOE_DICT_EST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_DICT_EST, TomoeDictEst))
#define TOMOE_DICT_EST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_DICT_EST, TomoeDictEstClass))
#define TOMOE_IS_DICT_EST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_DICT_EST))
#define TOMOE_IS_DICT_EST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_DICT_EST))
#define TOMOE_DICT_EST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_DICT_EST, TomoeDictEstClass))

enum {
    PROP_0,
    PROP_NAME,
    PROP_DATABASE_NAME,
    PROP_EDITABLE
};

typedef struct _TomoeDictEst TomoeDictEst;
typedef struct _TomoeDictEstClass TomoeDictEstClass;
struct _TomoeDictEst
{
    TomoeDict            object;
    gchar               *name;
    gchar               *database_name;

    gboolean             editable;

    ESTDB               *db;
};

struct _TomoeDictEstClass
{
    TomoeDictClass parent_class;
};

typedef struct _TomoeDictSearchContext {
    TomoeQuery *query;
    GList *results;
} TomoeDictSearchContext;

static GType tomoe_type_dict_est = 0;
static GObjectClass *parent_class;

static GObject     *constructor               (GType                  type,
                                               guint                  n_props,
                                               GObjectConstructParam *props);
static void         dispose                   (GObject       *object);
static void         set_property              (GObject       *object,
                                               guint         prop_id,
                                               const GValue  *value,
                                               GParamSpec    *pspec);
static void         get_property              (GObject       *object,
                                               guint          prop_id,
                                               GValue        *value,
                                               GParamSpec    *pspec);
static const gchar *get_name                  (TomoeDict     *dict);
static gboolean     register_char             (TomoeDict     *dict,
                                               TomoeChar     *chr);
static gboolean     unregister_char           (TomoeDict     *dict,
                                               const gchar   *utf8);
static TomoeChar   *get_char                  (TomoeDict     *dict,
                                               const gchar   *utf8);
static GList       *search                    (TomoeDict     *dict,
                                               TomoeQuery    *query);
static gboolean     flush                     (TomoeDict     *dict);
static gboolean     tomoe_dict_est_open       (TomoeDictEst  *dict);
static gboolean     tomoe_dict_est_close      (TomoeDictEst  *dict);

static void
class_init (TomoeDictEstClass *klass)
{
    GObjectClass *gobject_class;
    TomoeDictClass *dict_class;

    parent_class = g_type_class_peek_parent (klass);

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructor  = constructor;
    gobject_class->dispose      = dispose;
    gobject_class->set_property = set_property;
    gobject_class->get_property = get_property;

    dict_class = TOMOE_DICT_CLASS (klass);
    dict_class->get_name        = get_name;
    dict_class->register_char   = register_char;
    dict_class->unregister_char = unregister_char;
    dict_class->get_char        = get_char;
    dict_class->search          = search;
    dict_class->flush           = flush;

    g_object_class_install_property (
        gobject_class,
        PROP_NAME,
        g_param_spec_string (
            "name",
            "Name",
            "The name of the dictionary",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (
        gobject_class,
        PROP_DATABASE_NAME,
        g_param_spec_string (
            "database_name",
            "Database name",
            "The name of Hyper Estraier database",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property(
        gobject_class,
        PROP_EDITABLE,
        g_param_spec_boolean(
            "editable",
            "Editable",
            "Editable flag",
            TRUE,
            G_PARAM_READWRITE));
}

static void
init (TomoeDictEst *dict)
{
    dict->name          = NULL;
    dict->database_name = NULL;
    dict->db            = NULL;
}

static void
register_type (GTypeModule *type_module)
{
    static const GTypeInfo info =
        {
            sizeof (TomoeDictEstClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (TomoeDictEst),
            0,
            (GInstanceInitFunc) init,
        };

    tomoe_type_dict_est = g_type_module_register_type (type_module,
                                                       TOMOE_TYPE_DICT,
                                                       "TomoeDictEst",
                                                       &info, 0);
}

G_MODULE_EXPORT void
TOMOE_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    register_type (type_module);
}

G_MODULE_EXPORT void
TOMOE_MODULE_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
TOMOE_MODULE_IMPL_INSTANTIATE (const gchar *first_property, va_list var_args)
{
    return g_object_new_valist (TOMOE_TYPE_DICT_EST, first_property, var_args);
}

static GObject *
constructor (GType type, guint n_props,
             GObjectConstructParam *props)
{
    GObject *object;
    GObjectClass *klass = G_OBJECT_CLASS (parent_class);
    TomoeDictEst *dict;

    object = klass->constructor (type, n_props, props);
    dict = TOMOE_DICT_EST (object);

    tomoe_dict_est_open (dict);

    return object;
}

static void
set_property (GObject *object,
              guint prop_id,
              const GValue *value,
              GParamSpec *pspec)
{
    TomoeDictEst *dict = TOMOE_DICT_EST (object);

    switch (prop_id) {
      case PROP_NAME:
        dict->name = g_value_dup_string (value);
        break;
      case PROP_DATABASE_NAME:
        dict->database_name = g_value_dup_string (value);
        break;
      case PROP_EDITABLE:
        dict->editable = g_value_get_boolean (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}


static void
get_property (GObject *object,
              guint prop_id,
              GValue *value,
              GParamSpec *pspec)
{
    TomoeDictEst *dict = TOMOE_DICT_EST (object);

    switch (prop_id) {
      case PROP_NAME:
        g_value_set_string (value, dict->name);
        break;
      case PROP_DATABASE_NAME:
        g_value_set_string (value, dict->database_name);
        break;
      case PROP_EDITABLE:
        g_value_set_boolean (value, dict->editable);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
dispose (GObject *object)
{
    TomoeDictEst *dict;

    dict = TOMOE_DICT_EST (object);

    tomoe_dict_est_close (dict);

    if (dict->name)
        g_free (dict->name);
    if (dict->database_name)
        g_free (dict->database_name);

    dict->name          = NULL;
    dict->database_name = NULL;

    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static const gchar*
get_name (TomoeDict *_dict)
{
    TomoeDictEst *dict = TOMOE_DICT_EST (_dict);
    g_return_val_if_fail (TOMOE_IS_DICT_EST (dict), NULL);
    return dict->name;
}

static gboolean
register_char (TomoeDict *_dict, TomoeChar *chr)
{
    TomoeDictEst *dict = TOMOE_DICT_EST (_dict);
    gboolean success = FALSE;
    const gchar *original_value;
    gchar *value;
    gint n_strokes;
    ESTDOC *doc;

    g_return_val_if_fail (TOMOE_IS_DICT_EST (dict), success);
    g_return_val_if_fail (chr, success);

    doc = est_doc_new ();

    original_value = tomoe_char_get_utf8 (chr);
    if (original_value) {
        value = g_strdup_printf ("font:%s", original_value);
        est_doc_add_attr (doc, "@uri", value);
        g_free (value);

        est_doc_add_attr (doc, "utf8", original_value);
    }

    n_strokes = tomoe_char_get_n_strokes (chr);
    if (n_strokes < 0) {
        TomoeWriting *writing;
        writing = tomoe_char_get_writing (chr);
        if (writing)
            n_strokes = tomoe_writing_get_n_strokes (writing);
    }

    if (n_strokes >= 0) {
        value = g_strdup_printf ("%d", n_strokes);
        est_doc_add_attr (doc, "n_strokes", value);
        g_free (value);
    }

    original_value = tomoe_char_get_variant (chr);
    if (original_value) {
        est_doc_add_attr (doc, "variant", original_value);
    }

    value = tomoe_char_to_xml (chr);
    est_doc_add_hidden_text (doc, value);
    g_free (value);

    success = est_db_put_doc (dict->db, doc, ESTPDCLEAN);
    if (!success) {
        g_warning ("put error: %s\n", est_err_msg (est_db_error (dict->db)));
    }

    est_doc_delete (doc);

    return success;
}

static gboolean
unregister_char (TomoeDict *_dict, const gchar *utf8)
{
    TomoeDictEst *dict = TOMOE_DICT_EST (_dict);
    gboolean success = FALSE;
    ESTCOND *cond;
    int i, *results, n_results;
    gchar *expr;

    g_return_val_if_fail (TOMOE_IS_DICT_EST (dict), success);
    g_return_val_if_fail (utf8 && *utf8 != '\0', success);

    cond = est_cond_new ();
    expr = g_strdup_printf ("utf8 STREQ %s", utf8);
    est_cond_add_attr (cond, expr);
    g_free (expr);

    results = est_db_search (dict->db, cond, &n_results, NULL);

    for (i = 0; i < n_results; i++) {
        success = est_db_out_doc (dict->db, results[i], ESTODCLEAN);
        if (!success) {
            g_warning ("out error: %s\n",
                       est_err_msg (est_db_error (dict->db)));
            break;
        }
    }
    g_free (results);

    return success;
}

static TomoeChar *
get_char (TomoeDict *_dict, const gchar *utf8)
{
    TomoeDictEst *dict = TOMOE_DICT_EST (_dict);
    TomoeChar *chr = NULL;
    ESTCOND *cond;
    int i, *results, n_results;
    gchar *expr;

    g_return_val_if_fail (TOMOE_IS_DICT_EST (dict), chr);
    g_return_val_if_fail (utf8 && *utf8 != '\0', chr);

    cond = est_cond_new ();
    expr = g_strdup_printf ("utf8 STREQ %s", utf8);
    est_cond_add_attr (cond, expr);
    g_free (expr);

    results = est_db_search (dict->db, cond, &n_results, NULL);

    for (i = 0; i < n_results; i++) {
        ESTDOC *doc;
        const gchar *xml;

        doc = est_db_get_doc (dict->db, results[i], 0);
        if (!doc) continue;

        xml = est_doc_hidden_texts (doc);
        if (xml)
            chr = tomoe_char_new_from_xml_data (xml, -1);

        est_doc_delete (doc);

        if (chr) break;
    }
    g_free (results);

    return chr;
}

static GList *
search (TomoeDict *_dict, TomoeQuery *query)
{
    TomoeDictEst *dict = TOMOE_DICT_EST (_dict);
    GList *candidates = NULL;
    TomoeReading *reading;
    ESTCOND *cond;
    gchar *expr;
    int i, *results, n_results;
    gint min_n_strokes, max_n_strokes;

    g_return_val_if_fail (TOMOE_IS_DICT_EST (dict), candidates);

    cond = est_cond_new ();
    est_cond_set_order (cond, "utf8 STRA");

    min_n_strokes = tomoe_query_get_min_n_strokes (query);
    if (min_n_strokes >= 0) {
        expr = g_strdup_printf ("n_strokes NUMGE %d", min_n_strokes);
        est_cond_add_attr (cond, expr);
        g_free (expr);
    }

    max_n_strokes = tomoe_query_get_max_n_strokes (query);
    if (max_n_strokes >= 0) {
        expr = g_strdup_printf ("n_strokes NUMLE %d", max_n_strokes);
        est_cond_add_attr (cond, expr);
        g_free (expr);
    }

    reading = g_list_nth_data ((GList *)tomoe_query_get_readings (query), 0);
    if (reading) {
        expr = tomoe_reading_to_xml (reading);
        est_cond_set_phrase (cond, expr);
        g_free (expr);
    }

    results = est_db_search (dict->db, cond, &n_results, NULL);
    for (i = 0; i < n_results; i++) {
        ESTDOC *doc;
        const gchar *xml;

        doc = est_db_get_doc (dict->db, results[i], 0);
        if (!doc) continue;

        xml = est_doc_hidden_texts (doc);
        if (xml) {
            TomoeChar *chr;
            chr = tomoe_char_new_from_xml_data (xml, -1);
            candidates = g_list_prepend (candidates, tomoe_candidate_new (chr));
            g_object_unref (chr);
        }

        est_doc_delete (doc);
    }
    g_free (results);

    return candidates;
}

static gboolean
flush (TomoeDict *_dict)
{
    TomoeDictEst *dict = TOMOE_DICT_EST (_dict);

    return est_db_sync (dict->db);
}

static gboolean
tomoe_dict_est_open (TomoeDictEst *dict)
{
    gboolean success = TRUE;
    int ecode;

    dict->db = est_db_open (dict->database_name, ESTDBWRITER | ESTDBCREAT,
                            &ecode);

    if (!dict->db) {
        g_warning ("open error: %s\n", est_err_msg (ecode));
        success = FALSE;
    }

    return success;
}

static gboolean
tomoe_dict_est_close (TomoeDictEst *dict)
{
    gboolean success;
    int ecode;

    success = est_db_close (dict->db, &ecode);
    if (!success) {
        g_warning ("close error: %s\n", est_err_msg (ecode));
    }

    dict->db = NULL;

    return success;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/