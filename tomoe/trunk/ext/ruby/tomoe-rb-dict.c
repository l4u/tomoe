#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TDIC(obj)

#define TOMOE_DICT_PREFIX "TomoeDict"
#define TOMOE_DICT_PREFIX_LEN (strlen(TOMOE_DICT_PREFIX))

static VALUE cTomoeDict;

static VALUE
td_s_load(VALUE self, VALUE base_dir)
{
    GList *registered_types, *node;

    tomoe_dict_load(NIL_P(base_dir) ? NULL : RVAL2CSTR(base_dir));

    registered_types = tomoe_dict_get_registered_types ();
    for (node = registered_types; node; node = g_list_next (node)) {
        const gchar *name = node->data;
        GType type;

        type = g_type_from_name (name);
        if (type && g_str_has_prefix (name, TOMOE_DICT_PREFIX)) {
            G_DEF_CLASS(type, name + TOMOE_DICT_PREFIX_LEN, cTomoeDict);
        }
    }
    g_list_free (registered_types);

    return Qnil;
}

static VALUE
td_s_unload(VALUE self)
{
    GList *registered_types, *node;

    registered_types = tomoe_dict_get_registered_types ();
    for (node = registered_types; node; node = g_list_next (node)) {
        const gchar *name = node->data;

        if (g_str_has_prefix(name, TOMOE_DICT_PREFIX)) {
            const gchar *klass_name = name + TOMOE_DICT_PREFIX_LEN;
            if (rb_const_defined (cTomoeDict, rb_intern(klass_name))) {
                rb_mod_remove_const (cTomoeDict, rb_str_new2(klass_name));
            }
        }
    }
    g_list_free (registered_types);

    tomoe_dict_unload();
    return Qnil;
}

static VALUE
td_get_char(VALUE self, VALUE utf8)
{
    return GOBJ2RVALU(tomoe_dict_get_char(_SELF(self), RVAL2CSTR(utf8)));
}

static VALUE
td_register_char(VALUE self, VALUE chr)
{
    return CBOOL2RVAL(tomoe_dict_register_char(_SELF(self), RVAL2TCHR(chr)));
}

static VALUE
td_unregister_char(VALUE self, VALUE utf8)
{
    return CBOOL2RVAL(tomoe_dict_unregister_char(_SELF(self), RVAL2CSTR(utf8)));
}

static VALUE
td_search(VALUE self, VALUE query)
{
    return GLIST2ARYF(tomoe_dict_search(_SELF(self), RVAL2TQRY(query)));
}

static VALUE
td_flush(VALUE self)
{
    return CBOOL2RVAL(tomoe_dict_flush(_SELF(self)));
}

void
Init_tomoe_dict(VALUE mTomoe)
{
    cTomoeDict = G_DEF_CLASS(TOMOE_TYPE_DICT, "Dict", mTomoe);

    rb_define_singleton_method(cTomoeDict, "load", td_s_load, 1);
    rb_define_singleton_method(cTomoeDict, "unload", td_s_unload, 0);

    rb_define_method(cTomoeDict, "[]", td_get_char, 1);
    rb_define_method(cTomoeDict, "register", td_register_char, 1);
    rb_define_method(cTomoeDict, "unregister", td_unregister_char, 1);

    rb_define_method(cTomoeDict, "search", td_search, 1);

    rb_define_method(cTomoeDict, "flush", td_flush, 0);
}
