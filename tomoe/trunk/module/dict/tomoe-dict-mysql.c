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
#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gmodule.h>

#include <mysql.h>

#include <tomoe-module-impl.h>
#include <tomoe-dict.h>
#include <tomoe-candidate.h>
#include <glib-utils.h>

#define TOMOE_TYPE_DICT_MYSQL            tomoe_type_dict_mysql
#define TOMOE_DICT_MYSQL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_DICT_MYSQL, TomoeDictMySQL))
#define TOMOE_DICT_MYSQL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_DICT_MYSQL, TomoeDictMySQLClass))
#define TOMOE_IS_DICT_MYSQL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_DICT_MYSQL))
#define TOMOE_IS_DICT_MYSQL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_DICT_MYSQL))
#define TOMOE_DICT_MYSQL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TOMOE_TYPE_DICT_MYSQL, TomoeDictMySQLClass))

#define TOMOE_DICT_MYSQL_IS_CONNECTED(dict) (dict->mysql)

enum {
    PROP_0,
    PROP_NAME,
    PROP_EDITABLE,
    PROP_HOST,
    PROP_USER,
    PROP_PASSWORD,
    PROP_DATABASE,
    PROP_PORT,
    PROP_SOCKET
};

typedef struct _TomoeDictMySQL TomoeDictMySQL;
typedef struct _TomoeDictMySQLClass TomoeDictMySQLClass;
struct _TomoeDictMySQL
{
    TomoeDict            object;
    gchar               *name;

    gboolean             editable;

    MYSQL               *mysql;

    char                *host;
    char                *user;
    char                *password;
    char                *database;
    unsigned int         port;
    char                *socket;
};

struct _TomoeDictMySQLClass
{
    TomoeDictClass parent_class;
};

typedef struct _RegisterMetaDataContext RegisterMetaDataContext;
struct _RegisterMetaDataContext
{
    TomoeDictMySQL *dict;
    const gchar    *utf8;
    gboolean        success;
};


static GType tomoe_type_dict_mysql = 0;
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
static gboolean     is_editable               (TomoeDict     *dict);
static gchar       *get_available_private_utf8 (TomoeDict    *dict);
static gboolean     tomoe_dict_mysql_connect  (TomoeDictMySQL  *dict);
static gboolean     tomoe_dict_mysql_close    (TomoeDictMySQL  *dict);

static void
class_init (TomoeDictMySQLClass *klass)
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
    dict_class->is_editable     = is_editable;
    dict_class->get_available_private_utf8 = get_available_private_utf8;

    g_object_class_install_property (
        gobject_class,
        PROP_NAME,
        g_param_spec_string (
            "name",
            "Name",
            "The name of the dictionary",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(
        gobject_class,
        PROP_EDITABLE,
        g_param_spec_boolean(
            "editable",
            "Editable",
            "Editable flag",
            FALSE,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (
        gobject_class,
        PROP_HOST,
        g_param_spec_string (
            "host",
            "Host",
            "The host name of MySQL server",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (
        gobject_class,
        PROP_USER,
        g_param_spec_string (
            "user",
            "User",
            "The user name to connect MySQL server",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (
        gobject_class,
        PROP_PASSWORD,
        g_param_spec_string (
            "password",
            "Password",
            "The password to connect MySQL server",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (
        gobject_class,
        PROP_DATABASE,
        g_param_spec_string (
            "database",
            "Database",
            "The database name of MySQL server",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (
        gobject_class,
        PROP_PORT,
        g_param_spec_uint (
            "port",
            "Port",
            "The port number to connect MySQL server",
            0, 65535, 0,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (
        gobject_class,
        PROP_SOCKET,
        g_param_spec_string (
            "socket",
            "Socket",
            "The socket file name of MySQL server",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
init (TomoeDictMySQL *dict)
{
    dict->name          = NULL;
    dict->editable      = FALSE;

    dict->mysql         = NULL;

    dict->host          = NULL;
    dict->user          = NULL;
    dict->password      = NULL;
    dict->database      = NULL;
    dict->port          = 0;
    dict->socket        = NULL;
}

static void
register_type (GTypeModule *type_module)
{
    static const GTypeInfo info =
        {
            sizeof (TomoeDictMySQLClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (TomoeDictMySQL),
            0,
            (GInstanceInitFunc) init,
        };

    tomoe_type_dict_mysql = g_type_module_register_type (type_module,
                                                       TOMOE_TYPE_DICT,
                                                       "TomoeDictMySQL",
                                                       &info, 0);
}

G_MODULE_EXPORT GList *
TOMOE_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    GList *registered_types = NULL;

    register_type (type_module);
    if (tomoe_type_dict_mysql)
        registered_types =
            g_list_prepend (registered_types,
                            (gchar *) g_type_name (tomoe_type_dict_mysql));

    return registered_types;
}

G_MODULE_EXPORT void
TOMOE_MODULE_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
TOMOE_MODULE_IMPL_INSTANTIATE (const gchar *first_property, va_list var_args)
{
    return g_object_new_valist (TOMOE_TYPE_DICT_MYSQL, first_property, var_args);
}

static GObject *
constructor (GType type, guint n_props,
             GObjectConstructParam *props)
{
    GObject *object;
    GObjectClass *klass = G_OBJECT_CLASS (parent_class);
    TomoeDictMySQL *dict;

    object = klass->constructor (type, n_props, props);
    dict = TOMOE_DICT_MYSQL (object);

    tomoe_dict_mysql_connect (dict);

    return object;
}

static void
set_property (GObject *object,
              guint prop_id,
              const GValue *value,
              GParamSpec *pspec)
{
    TomoeDictMySQL *dict = TOMOE_DICT_MYSQL (object);

    switch (prop_id) {
      case PROP_NAME:
        g_free (dict->name);
        dict->name = g_value_dup_string (value);
        break;
      case PROP_EDITABLE:
        dict->editable = g_value_get_boolean (value);
        break;
      case PROP_HOST:
        g_free (dict->host);
        dict->host = g_value_dup_string (value);
        break;
      case PROP_USER:
        g_free (dict->user);
        dict->user = g_value_dup_string (value);
        break;
      case PROP_PASSWORD:
        g_free (dict->password);
        dict->password = g_value_dup_string (value);
        break;
      case PROP_DATABASE:
        g_free (dict->database);
        dict->database = g_value_dup_string (value);
        break;
      case PROP_PORT:
        dict->port = g_value_get_uint (value);
        break;
      case PROP_SOCKET:
        g_free (dict->socket);
        dict->socket = g_value_dup_string (value);
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
    TomoeDictMySQL *dict = TOMOE_DICT_MYSQL (object);

    switch (prop_id) {
      case PROP_NAME:
        g_value_set_string (value, dict->name);
        break;
      case PROP_EDITABLE:
        g_value_set_boolean (value, dict->editable);
        break;
      case PROP_HOST:
        g_value_set_string (value, dict->host);
        break;
      case PROP_USER:
        g_value_set_string (value, dict->user);
        break;
      case PROP_PASSWORD:
        g_value_set_string (value, dict->password);
        break;
      case PROP_DATABASE:
        g_value_set_string (value, dict->database);
        break;
      case PROP_PORT:
        g_value_set_uint (value, dict->port);
        break;
      case PROP_SOCKET:
        g_value_set_string (value, dict->socket);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
dispose (GObject *object)
{
    TomoeDictMySQL *dict;

    dict = TOMOE_DICT_MYSQL (object);

    tomoe_dict_mysql_close (dict);

    if (dict->name)
        g_free (dict->name);
    if (dict->host)
        g_free (dict->host);
    if (dict->user)
        g_free (dict->user);
    if (dict->password)
        g_free (dict->password);
    if (dict->database)
        g_free (dict->database);
    if (dict->socket)
        g_free (dict->socket);

    dict->name          = NULL;
    dict->host          = NULL;
    dict->user          = NULL;
    dict->password      = NULL;
    dict->database      = NULL;
    dict->socket        = NULL;

    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static const gchar*
get_name (TomoeDict *_dict)
{
    TomoeDictMySQL *dict = TOMOE_DICT_MYSQL (_dict);
    g_return_val_if_fail (TOMOE_IS_DICT_MYSQL (dict), NULL);
    if (dict->name)
        return dict->name;
    else
        return "MySQL";
}

static gboolean
execute_query (TomoeDictMySQL *dict, const gchar *sql)
{
    int status;

    status = mysql_query (dict->mysql, sql);
    if (status) {
        g_warning ("failed to execute query: %s: %s",
                   sql, mysql_error (dict->mysql));
        return FALSE;
    } else {
        return TRUE;
    }
}

static void
append_string_value (TomoeDictMySQL *dict, GString *sql, const gchar *value)
{
    gint len;
    gchar *escaped_value;

    len = strlen (value);
    escaped_value = g_new (gchar, len * 2 + 1);
    mysql_real_escape_string (dict->mysql, escaped_value, value, len);
    g_string_append_printf (sql, "'%s'", escaped_value);
    g_free (escaped_value);
}

static gboolean
register_char_readings (TomoeDictMySQL *dict, const gchar *utf8,
                        const GList *readings)
{
    GString *sql;
    GList *node;
    gboolean success = TRUE;

    sql = g_string_new ("");
    for (node = (GList *)readings; node; node = g_list_next (node)) {
        TomoeReading *reading = node->data;

        g_string_assign (sql,
                         "INSERT INTO readings "            \
                         "(utf8, reading_type, reading) "   \
                         "VALUES (");
        append_string_value (dict, sql, utf8);
        g_string_append_printf (sql, ", %d, ",
                                tomoe_reading_get_reading_type (reading));
        append_string_value (dict, sql, tomoe_reading_get_reading (reading));
        g_string_append (sql, ")");

        success = execute_query (dict, sql->str);
        if (!success)
            break;
    }
    g_string_free (sql, TRUE);

    return success;
}

static gboolean
register_char_radicals (TomoeDictMySQL *dict, const gchar *utf8,
                        const GList *radicals)
{
    GString *sql;
    GList *node;
    gboolean success = TRUE;

    sql = g_string_new ("");
    for (node = (GList *)radicals; node; node = g_list_next (node)) {
        const gchar *radical = node->data;

        g_string_assign (sql, "INSERT INTO radicals (utf8, radical) VALUES (");
        append_string_value (dict, sql, utf8);
        g_string_append (sql, ", ");
        append_string_value (dict, sql, radical);
        g_string_append (sql, ")");

        success = execute_query (dict, sql->str);
        if (!success)
            break;
    }
    g_string_free (sql, TRUE);

    return success;
}

static void
register_char_meta_datum (gpointer _key, gpointer _value, gpointer user_data)
{
    const gchar *key = _key;
    const gchar *value = _value;
    RegisterMetaDataContext *context = user_data;

    if (context->success) {
        GString *sql;

        sql = g_string_new ("INSERT INTO meta_data "    \
                            "(utf8, name, value) "      \
                            "VALUES (");
        append_string_value (context->dict, sql, context->utf8);
        g_string_append (sql, ", ");
        append_string_value (context->dict, sql, key);
        g_string_append (sql, ", ");
        append_string_value (context->dict, sql, value);
        g_string_append (sql, ")");

        context->success = execute_query (context->dict, sql->str);
        g_string_free (sql, TRUE);
    }
}

static gboolean
register_char_meta_data (TomoeDictMySQL *dict, const gchar *utf8,
                         TomoeChar *chr)
{
    RegisterMetaDataContext context;

    context.dict = dict;
    context.utf8 = utf8;
    context.success = TRUE;

    tomoe_char_meta_data_foreach (chr, register_char_meta_datum, &context);

    return context.success;
}

static gboolean
register_char (TomoeDict *_dict, TomoeChar *chr)
{
    TomoeDictMySQL *dict = TOMOE_DICT_MYSQL (_dict);
    GString *sql;
    gint n_strokes;
    const gchar *variant;
    const gchar *utf8;
    gboolean success = FALSE;

    g_return_val_if_fail (TOMOE_IS_DICT_MYSQL (dict), FALSE);
    g_return_val_if_fail (TOMOE_IS_CHAR (chr), FALSE);

    g_return_val_if_fail (TOMOE_DICT_MYSQL_IS_CONNECTED (dict), FALSE);

    if (!execute_query (dict, "START TRANSACTION"))
        return FALSE;

    sql = g_string_new ("INSERT INTO chars "            \
                        "(utf8, n_strokes, variant) "   \
                        "VALUES (");

    utf8 = tomoe_char_get_utf8 (chr);
    append_string_value (dict, sql, utf8);

    n_strokes = tomoe_char_get_n_strokes (chr);
    if (n_strokes >= 0)
        g_string_append_printf (sql, ", %d, ", n_strokes);
    else
        g_string_append (sql, ", NULL, ");

    variant = tomoe_char_get_variant (chr);
    if (variant)
        g_string_append_printf (sql, "%d)", g_utf8_get_char (variant));
    else
        g_string_append (sql, "NULL)");

    success = execute_query (dict, sql->str);
    g_string_free (sql, TRUE);

    if (success &&
        register_char_readings (dict, utf8, tomoe_char_get_readings (chr)) &&
        register_char_radicals (dict, utf8, tomoe_char_get_radicals (chr)) &&
        register_char_meta_data (dict, utf8, chr)) {
        return execute_query (dict, "COMMIT");
    } else {
        return FALSE;
    }
}

static gboolean
unregister_char (TomoeDict *_dict, const gchar *utf8)
{
    TomoeDictMySQL *dict = TOMOE_DICT_MYSQL (_dict);
    GString *sql;
    gboolean success;

    g_return_val_if_fail (TOMOE_IS_DICT_MYSQL (dict), FALSE);
    g_return_val_if_fail (utf8 && *utf8 != '\0', FALSE);

    g_return_val_if_fail (TOMOE_DICT_MYSQL_IS_CONNECTED (dict), FALSE);

    if (!execute_query (dict, "START TRANSACTION"))
        return FALSE;

    sql = g_string_new ("DELETE FROM chars WHERE utf8 = ");
    append_string_value (dict, sql, utf8);
    success = execute_query (dict, sql->str);
    if (!success) goto done;

    g_string_assign (sql, "DELETE FROM readings WHERE utf8 = ");
    append_string_value (dict, sql, utf8);
    success = execute_query (dict, sql->str);
    if (!success) goto done;

    g_string_assign (sql, "DELETE FROM radicals WHERE utf8 = ");
    append_string_value (dict, sql, utf8);
    success = execute_query (dict, sql->str);
    if (!success) goto done;

    g_string_assign (sql, "DELETE FROM meta_data WHERE utf8 = ");
    append_string_value (dict, sql, utf8);
    success = execute_query (dict, sql->str);
    if (!success) goto done;

  done:
    g_string_free (sql, TRUE);

    if (success)
        return execute_query (dict, "COMMIT");
    else
        return FALSE;
}

static void
generate_sql_condition_utf8 (TomoeDictMySQL *dict, GString *sql,
                             TomoeQuery *query)
{
    const gchar *utf8;

    utf8 = tomoe_query_get_utf8 (query);
    if (utf8) {
        g_string_append (sql, " AND chars.utf8 = ");
        append_string_value (dict, sql, utf8);
    }
}

static void
generate_sql_condition_min_n_strokes (TomoeDictMySQL *dict, GString *sql,
                                      TomoeQuery *query)
{
    gint min_n_strokes;

    min_n_strokes = tomoe_query_get_min_n_strokes (query);
    if (min_n_strokes >= 0) {
        g_string_append_printf (sql, " AND n_strokes >= %d",
                                min_n_strokes);
    }
}

static void
generate_sql_condition_max_n_strokes (TomoeDictMySQL *dict, GString *sql,
                                      TomoeQuery *query)
{
    gint max_n_strokes;

    max_n_strokes = tomoe_query_get_max_n_strokes (query);
    if (max_n_strokes >= 0) {
        g_string_append_printf (sql, " AND n_strokes <= %d",
                                max_n_strokes);
    }
}

static void
generate_sql_condition_readings (TomoeDictMySQL *dict, GString *sql,
                                 TomoeQuery *query)
{
    GList *node;

    for (node = (GList *)tomoe_query_get_readings (query);
         node;
         node = g_list_next (node)) {
        TomoeReading *tomoe_reading = node->data;
        TomoeReadingType reading_type;
        const gchar *reading;

        reading_type = tomoe_reading_get_reading_type (tomoe_reading);
        if (reading_type != TOMOE_READING_INVALID)
            g_string_append_printf (sql, " AND reading_type = %d",
                                    reading_type);

        reading = tomoe_reading_get_reading (tomoe_reading);
        if (reading) {
            g_string_append (sql, " AND reading = ");
            append_string_value (dict, sql, reading);
        }
    }
}


static gchar *
generate_sql (TomoeDictMySQL *dict, TomoeQuery *query)
{
    GString *sql;

    sql = g_string_new ("SELECT "                                   \
                        "chars.utf8 AS utf8, "                      \
                        "chars.n_strokes AS n_strokes, "            \
                        "chars.variant AS variant, "                \
                        "readings.id AS reading_id, "               \
                        "readings.reading_type AS reading_type, "   \
                        "readings.reading AS reading, "             \
                        "radicals.id AS radical_id, "               \
                        "radicals.radical_utf8 AS radical_utf8 "    \
                        "FROM chars "                               \
                        "LEFT OUTER JOIN readings "                 \
                        "ON chars.utf8 = readings.utf8 "            \
                        "LEFT OUTER JOIN radicals "                 \
                        "ON chars.utf8 = radicals.utf8 "            \
                        "WHERE TRUE = TRUE");

    generate_sql_condition_utf8 (dict, sql, query);
    generate_sql_condition_min_n_strokes (dict, sql, query);
    generate_sql_condition_max_n_strokes (dict, sql, query);
    generate_sql_condition_readings (dict, sql, query);

    return g_string_free (sql, FALSE);
}

static GList *
retrieve_candidates (TomoeDictMySQL *dict, MYSQL_RES *result)
{
    GList *results = NULL;
    TomoeChar *chr = NULL;
    GArray *reading_ids = NULL, *radical_ids = NULL;
    gchar *prev_utf8 = NULL;
    MYSQL_ROW row;

    while ((row = mysql_fetch_row (result))) {
        gchar *utf8;
        gint n_strokes = -1;
        gchar *variant;
        gint reading_id = -1;
        TomoeReadingType reading_type = TOMOE_READING_INVALID;
        gchar *reading;
        gint radical_id = -1;
        gchar *radical;

        utf8 = row[0];
        if (row[1])
            n_strokes = atoi (row[1]);
        variant = row[2];
        if (row[3])
            reading_id = atoi (row[3]);
        if (row[4])
            reading_type = atoi (row[4]);
        reading = row[5];
        if (row[6])
            radical_id = atoi (row[6]);
        radical = row[7];

        if (chr && utf8 && prev_utf8 && strcmp (utf8, prev_utf8) != 0) {
            TomoeCandidate *cand;

            cand = tomoe_candidate_new (chr);
            results = g_list_prepend (results, cand);
            g_object_unref (chr);
            chr = NULL;
            g_free (prev_utf8);
            prev_utf8 = NULL;
            if (reading_ids)
                g_array_free (reading_ids, TRUE);
            if (radical_ids)
                g_array_free (radical_ids, TRUE);
            reading_ids = NULL;
            radical_ids = NULL;
        }

        if (!prev_utf8)
            prev_utf8 = g_strdup (utf8);

        if (!reading_ids)
            reading_ids = g_array_new (FALSE, TRUE, sizeof (gboolean));
        if (!radical_ids)
            radical_ids = g_array_new (FALSE, TRUE, sizeof (gboolean));

        if (!chr) {
            chr = tomoe_char_new ();
            tomoe_char_set_utf8 (chr, utf8);
            tomoe_char_set_n_strokes (chr, n_strokes);
            tomoe_char_set_variant (chr, variant);
        }

        if (chr && reading_id >= 0 &&
            (reading_ids->len <= reading_id ||
             !reading_ids->data[reading_id])) {
            TomoeReading *tomoe_reading;

            if (reading_ids->len <= reading_id) {
                g_array_set_size (reading_ids, reading_id);
            }
            reading_ids->data[reading_id] = TRUE;

            tomoe_reading = tomoe_reading_new (reading_type, reading);
            tomoe_char_add_reading (chr, tomoe_reading);
            g_object_unref (tomoe_reading);
        }

        if (chr && radical_id >= 0 &&
            (radical_ids->len <= radical_id ||
             !radical_ids->data[radical_id])) {
            if (radical_ids->len <= radical_id) {
                g_array_set_size (radical_ids, radical_id);
            }
            radical_ids->data[radical_id] = TRUE;
            tomoe_char_add_radical (chr, radical);
        }
    }

    if (chr) {
        TomoeCandidate *cand;

        cand = tomoe_candidate_new (chr);
        results = g_list_prepend (results, cand);
        g_object_unref (chr);
    }

    if (prev_utf8)
        g_free (prev_utf8);
    if (reading_ids)
        g_array_free (reading_ids, TRUE);
    if (radical_ids)
        g_array_free (radical_ids, TRUE);

    return results;
}

static TomoeChar *
get_char (TomoeDict *_dict, const gchar *utf8)
{
    TomoeDictMySQL *dict = TOMOE_DICT_MYSQL (_dict);
    TomoeChar *chr = NULL;
    TomoeQuery *query;
    GList *cands;

    g_return_val_if_fail (TOMOE_IS_DICT_MYSQL (dict), chr);
    g_return_val_if_fail (utf8 && utf8[0] != '\0', chr);

    g_return_val_if_fail (TOMOE_DICT_MYSQL_IS_CONNECTED (dict), chr);

    query = tomoe_query_new ();
    tomoe_query_set_utf8 (query, utf8);
    cands = search (_dict, query);
    g_object_unref (query);

    if (cands) {
        chr = g_object_ref (tomoe_candidate_get_char (cands->data));
        g_list_foreach (cands, (GFunc) g_object_unref, NULL);
        g_list_free (cands);
    }

    return chr;
}

static GList *
search (TomoeDict *_dict, TomoeQuery *query)
{
    TomoeDictMySQL *dict = TOMOE_DICT_MYSQL (_dict);
    gchar *sql;
    MYSQL_RES *result;
    GList *results = NULL;
    gboolean success;

    g_return_val_if_fail (TOMOE_IS_DICT_MYSQL (dict), results);
    g_return_val_if_fail (TOMOE_IS_QUERY (query), results);

    g_return_val_if_fail (TOMOE_DICT_MYSQL_IS_CONNECTED (dict), results);

    sql = generate_sql (dict, query);
    success = execute_query (dict, sql);
    g_free (sql);
    if (!success)
        return NULL;

    result = mysql_use_result (dict->mysql);
    if (result) {
        results = retrieve_candidates (dict, result);
        mysql_free_result (result);
    }

    return results;
}

static gboolean
flush (TomoeDict *_dict)
{
    return TRUE;
}

static gboolean
is_editable (TomoeDict *_dict)
{
    TomoeDictMySQL *dict = TOMOE_DICT_MYSQL (_dict);

    g_return_val_if_fail (TOMOE_IS_DICT_MYSQL (dict), FALSE);

    return dict->editable;
}

static gchar *
get_available_private_utf8 (TomoeDict *_dict)
{
    TomoeDictMySQL *dict = TOMOE_DICT_MYSQL (_dict);
    GString *sql;
    gchar *utf8;
    gchar pua_start_utf8[6];
    gint len;
    gboolean success;
    MYSQL_RES *result;

    g_return_val_if_fail (TOMOE_IS_DICT_MYSQL (dict), NULL);
    g_return_val_if_fail (TOMOE_DICT_MYSQL_IS_CONNECTED (dict), NULL);

    len = g_unichar_to_utf8 (TOMOE_CHAR_PRIVATE_USE_AREA_START, pua_start_utf8);
    pua_start_utf8[len] = '\0';

    sql = g_string_new ("SELECT utf8 FROM chars WHERE utf8 >= ");
    append_string_value (dict, sql, pua_start_utf8);
    g_string_append (sql, " ORDER BY utf8 LIMIT 1");

    success = execute_query (dict, sql->str);
    g_string_free (sql, TRUE);

    if (!success)
        return NULL;

    utf8 = NULL;
    result = mysql_store_result (dict->mysql);
    if (result) {
        MYSQL_ROW row;
        row = mysql_fetch_row (result);
        if (row) {
            utf8 = g_strdup (g_utf8_next_char (row[0]));
        } else {
            utf8 = g_strdup (pua_start_utf8);
        }
        mysql_free_result (result);
    }

    return utf8;
}

static gboolean
tomoe_dict_mysql_connect (TomoeDictMySQL *dict)
{
    tomoe_dict_mysql_close (dict);

    dict->mysql = mysql_init (NULL);
    if (!dict->mysql) {
        g_warning ("cannot initialize MySQL handler");
        return FALSE;
    }

    mysql_options (dict->mysql, MYSQL_READ_DEFAULT_GROUP, "TOMOE");
    mysql_options (dict->mysql, MYSQL_SET_CHARSET_NAME, "UTF8");
    if (!mysql_real_connect (dict->mysql, dict->host, dict->user,
                             dict->password, dict->database, dict->port,
                             dict->socket, 0)) {
        g_warning ("cannot connect to %s at %s:%d/%s by %s@XXX: %s",
                   dict->database, dict->host, dict->port,
                   dict->socket, dict->user, mysql_error (dict->mysql));
        tomoe_dict_mysql_close (dict);
        return FALSE;
    }

    return TRUE;
}

static gboolean
tomoe_dict_mysql_close (TomoeDictMySQL *dict)
{
    if (dict->mysql)
        mysql_close (dict->mysql);
    dict->mysql = NULL;

    return TRUE;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
