/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2007 Mathieu Blondel <mblondel@rubyforge.org>
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
 *  $ $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <tomoe-module-impl.h>
#include <tomoe-dict.h>
#include <tomoe-candidate.h>
#include <glib-utils.h>

#include "tomoe-dict-ptr-array.h"

#define TOMOE_TYPE_DICT_BINARY            tomoe_type_dict_binary
#define TOMOE_DICT_BINARY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
TOMOE_TYPE_DICT_BINARY, TomoeDictBinary))
#define TOMOE_DICT_BINARY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
TOMOE_TYPE_DICT_BINARY, TomoeDictBinaryClass))
#define TOMOE_IS_DICT_BINARY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
TOMOE_TYPE_DICT_BINARY))
#define TOMOE_IS_DICT_BINARY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
TOMOE_TYPE_DICT_BINARY))
#define TOMOE_DICT_BINARY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), \
TOMOE_TYPE_DICT_BINARY, TomoeDictBinaryClass))

enum {
    PROP_0,
    PROP_FILENAME
};

typedef struct _TomoeDictBinary TomoeDictBinary;
typedef struct _TomoeDictBinaryClass TomoeDictBinaryClass;
struct _TomoeDictBinary
{
    TomoeDictPtrArray    object;
    gchar               *filename;
    gchar               *name;
};

struct _TomoeDictBinaryClass
{
    TomoeDictPtrArrayClass parent_class;
};

static GType tomoe_type_dict_binary = 0;
static TomoeDictPtrArrayClass *parent_class;

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
static gboolean     flush                     (TomoeDict     *dict);
static gboolean     is_available              (TomoeDict     *dict);
static gboolean     tomoe_dict_binary_load       (TomoeDictBinary  *dict);
static gboolean     tomoe_dict_binary_save       (TomoeDictBinary  *dict);

static void
class_init (TomoeDictBinaryClass *klass)
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
    dict_class->flush           = flush;
    dict_class->is_available    = is_available;

    g_object_class_install_property (
        gobject_class,
        PROP_FILENAME,
        g_param_spec_string (
            "filename",
            "Filename",
            "The filename of binary file",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
init (TomoeDictBinary *dict)
{
    dict->filename = NULL;
    dict->name     = NULL;
}

static void
register_type (GTypeModule *type_module)
{
    static const GTypeInfo info =
        {
            sizeof (TomoeDictBinaryClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (TomoeDictBinary),
            0,
            (GInstanceInitFunc) init,
        };

    tomoe_type_dict_binary = g_type_module_register_type (type_module,
                                                       TOMOE_TYPE_DICT_PTR_ARRAY,
                                                       "TomoeDictBinary",
                                                       &info, 0);
}

G_MODULE_EXPORT GList *
TOMOE_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    GList *registered_types = NULL;

    register_type (type_module);
    if (tomoe_type_dict_binary)
        registered_types =
            g_list_prepend (registered_types,
                            (gchar *) g_type_name (tomoe_type_dict_binary));

    return registered_types;
}

G_MODULE_EXPORT void
TOMOE_MODULE_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
TOMOE_MODULE_IMPL_INSTANTIATE (const gchar *first_property, va_list args)
{
    return g_object_new_valist (TOMOE_TYPE_DICT_BINARY, first_property, args);
}

G_MODULE_EXPORT gchar *
TOMOE_MODULE_IMPL_GET_LOG_DOMAIN (void)
{
    return g_strdup (G_LOG_DOMAIN);
}

static GObject *
constructor (GType type, guint n_props,
             GObjectConstructParam *props)
{
    GObject *object;
    GObjectClass *klass = G_OBJECT_CLASS (parent_class);
    TomoeDictBinary *dict;

    object = klass->constructor (type, n_props, props);
    dict = TOMOE_DICT_BINARY (object);

    tomoe_dict_binary_load (dict);

    return object;
}

static void
set_property (GObject *object,
              guint prop_id,
              const GValue *value,
              GParamSpec *pspec)
{
    TomoeDictBinary *dict = TOMOE_DICT_BINARY (object);

    switch (prop_id) {
    case PROP_FILENAME:
        dict->filename = g_value_dup_string (value);
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
    TomoeDictBinary *dict = TOMOE_DICT_BINARY (object);

    switch (prop_id) {
    case PROP_FILENAME:
        g_value_set_string (value, dict->filename);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
dispose (GObject *object)
{
    TomoeDictBinary *dict;

    dict = TOMOE_DICT_BINARY(object);

    flush (TOMOE_DICT (dict));

    if (dict->name)
        g_free (dict->name);
    if (dict->filename)
        g_free (dict->filename);

    dict->name     = NULL;
    dict->filename = NULL;

    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static const gchar*
get_name (TomoeDict *_dict)
{
    TomoeDictBinary *dict = TOMOE_DICT_BINARY (_dict);
    g_return_val_if_fail (TOMOE_IS_DICT_BINARY (dict), NULL);
    return dict->name;
}

static gboolean
flush (TomoeDict *_dict)
{
    TomoeDictBinary *dict = TOMOE_DICT_BINARY (_dict);

    g_return_val_if_fail (TOMOE_IS_DICT_BINARY (dict), FALSE);

    if (!dict->filename)
        return FALSE;

    return tomoe_dict_binary_save (dict);
}

static gboolean
is_available (TomoeDict *_dict)
{
    TomoeDictBinary *dict = TOMOE_DICT_BINARY (_dict);

    g_return_val_if_fail (TOMOE_IS_DICT_BINARY (dict), FALSE);

    if (tomoe_dict_is_editable (_dict) && !dict->filename)
        return FALSE;

    if (!tomoe_dict_is_editable (_dict) && dict->filename &&
        !g_file_test (dict->filename, G_FILE_TEST_EXISTS))
        return FALSE;

    return TRUE;
}

static gushort
read_ushort (const guchar *d, gulong *c)
{
    gushort res = (gushort) ((d[*c] << 8) | d[*c+1]);
    *c = *c + 2;
    return res;
}

static gulong
read_ulong (const guchar *d, gulong *c)
{
    gulong res;
    res = (gulong) ((d[*c] << 24) | (d[*c+1] << 16) | (d[*c+2] << 8) |          
                    d[*c+3]);
    *c = *c + 4;
    return res;
}

static gchar*
read_string(const guchar *d, gushort l, gulong *c)
{
    gchar *res = g_strndup ((gchar *) &d[*c], l);
    *c = *c + l;
    return res;
}

static gboolean
tomoe_dict_binary_load (TomoeDictBinary *dict)
{
    gboolean success = TRUE;
    gchar *name, *utf8;
    const guchar *data;
    GPtrArray *chars;
    GMappedFile *file;
    gushort length, n_strokes, n_pairs, j, k, x, y;
    gulong n_chars, i;
    TomoeChar *chr;
    TomoeWriting *writing;
    gulong cursor = 0;

    if (!g_file_test (dict->filename, G_FILE_TEST_EXISTS))
        return success;

    name = NULL;
    chars = tomoe_dict_ptr_array_get_array (TOMOE_DICT_PTR_ARRAY (dict));

    /*
    Binary file format structure

    Dictionary title (offset 0) :
    - dictionary title length in bytes (unsigned short)
    - dictionary title in utf8

    Characters
    - number of characters in file (unsigned long)

    For each character :
    - length of character in bytes (unsigned short)
    - the character itself
    - number of strokes (unsigned short)

    For each stroke:
    - number of coordinate pairs (unsigned short)
    - the coordinates pairs (two unsigned short)
    */

    file = g_mapped_file_new (dict->filename, FALSE, NULL);
    data = (guchar *) g_mapped_file_get_contents (file);

    length = read_ushort (data, &cursor);
    if (length > 0)
        dict->name = read_string (data, length, &cursor);    

    n_chars = read_ulong (data, &cursor);
    for (i=0; i < n_chars; i++) {
        chr = tomoe_char_new ();

        length = read_ushort (data, &cursor);
        utf8 = read_string (data, length, &cursor);
        tomoe_char_set_utf8 (chr, utf8);
        g_free(utf8);

        n_strokes = read_ushort (data, &cursor);
        writing = tomoe_writing_new ();
        for (j=0; j < n_strokes; j++) {            
            n_pairs = read_ushort (data, &cursor);

            for (k=0; k < n_pairs; k++) {
                x = read_ushort (data, &cursor);
                y = read_ushort (data, &cursor);
                if (k == 0)
                    tomoe_writing_move_to (writing, x, y);
                else
                    tomoe_writing_line_to (writing, x, y);
            }            
        }
        tomoe_char_set_writing (chr, writing);

        g_ptr_array_add (chars, chr);
        chr = NULL;
    }

    g_mapped_file_free (file);

    tomoe_dict_ptr_array_sort (TOMOE_DICT_PTR_ARRAY (dict));

    return success;
}

static void
put_ushort (FILE *file, gushort val)
{
    putc ((val >> 8) & 0xff, file);
    putc (val & 0xff, file);
}

static void
put_ulong (FILE *file, gulong val) {
    putc ((val >> 24) & 0xff, file);
    putc ((val >> 16) & 0xff, file);
    putc ((val >> 8) & 0xff, file);
    putc (val & 0xff, file);
}

static gboolean
tomoe_dict_binary_save (TomoeDictBinary *dict)
{
    gboolean success = TRUE;
    gboolean modified = FALSE;
    GPtrArray *chars;
    FILE *file;
    gushort length;
    gulong i;
    guint n_strokes;
    TomoeChar *chr;
    TomoeWriting *writing;
    TomoePoint *p;
    const GList *stroke_list;
    GList *point_list;
    const gchar *utf8;

    g_return_val_if_fail (TOMOE_IS_DICT_BINARY (dict), FALSE);

    if (!tomoe_dict_is_editable (TOMOE_DICT (dict))) return FALSE;

    g_return_val_if_fail (dict->filename, FALSE);

    g_object_get (dict, "modified", &modified, NULL);
    if (!modified) return TRUE;

    file = g_fopen (dict->filename, "w");

    if (dict->name) {
        length = strlen (dict->name);
        put_ushort (file, length);
        fwrite (dict->name, 1, length, file);
    } else {
        put_ushort (file, 0);
    }

    chars = tomoe_dict_ptr_array_get_array (TOMOE_DICT_PTR_ARRAY (dict));
    put_ulong (file, chars->len);

    for (i = 0; i < chars->len; i++) {
        chr = g_ptr_array_index (chars, i);

        utf8 = tomoe_char_get_utf8 (chr);
        length = strlen (utf8);
        put_ushort (file, length);
        fwrite (utf8, 1, length, file);

        writing = tomoe_char_get_writing (chr);        
        n_strokes = tomoe_writing_get_n_strokes (writing);
        put_ushort (file, n_strokes);        

        stroke_list = tomoe_writing_get_strokes (writing);
        for (; stroke_list; stroke_list = g_list_next (stroke_list)) {
            point_list = stroke_list->data;

            if (!point_list) {
                put_ushort (file, 0);
            }
            else {
                put_ushort (file, g_list_length (point_list));

                for (; point_list; point_list = g_list_next (point_list)) {
                    p = point_list->data;
                    put_ushort (file, p->x);
                    put_ushort (file, p->y);
                }
            }
        }
    }

    fclose (file);

    g_object_set (dict, "modified", FALSE, NULL);

    return success;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
