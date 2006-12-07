#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TDIC(obj)

static VALUE
td_s_load(VALUE self, VALUE base_dir)
{
    tomoe_dict_load(NIL_P(base_dir) ? NULL : RVAL2CSTR(base_dir));
    return Qnil;
}

static VALUE
td_s_unload(VALUE self)
{
    tomoe_dict_unload();
    return Qnil;
}

static VALUE
td_s_new(VALUE self, VALUE rb_name, VALUE props)
{
    TomoeDict *dict;
    gchar *name;

    name = RVAL2CSTR(rb_name);
    if (strcmp(name, "xml") == 0) {
        VALUE filename, editable;
        filename = rb_hash_aref(props, CSTR2RVAL("filename"));
        editable = rb_hash_aref(props, CSTR2RVAL("editable"));
        dict = tomoe_dict_new(name,
                              "filename", RVAL2CSTR(filename),
                              "editable", RVAL2CBOOL(editable),
                              NULL);
    } else if (strcmp(name, "est") == 0) {
        VALUE rb_dict_name, database_name, editable;
        gchar *dict_name;

        rb_dict_name = rb_hash_aref(props, CSTR2RVAL("name"));
        dict_name = NIL_P(rb_dict_name) ? NULL : RVAL2CSTR(rb_dict_name);
        database_name = rb_hash_aref(props, CSTR2RVAL("database_name"));
        editable = rb_hash_aref(props, CSTR2RVAL("editable"));

        dict = tomoe_dict_new(name,
                              "name", dict_name,
                              "database_name", RVAL2CSTR(database_name),
                              "editable", RVAL2CBOOL(editable),
                              NULL);
    } else if (strcmp(name, "unihan") == 0) {
        dict = tomoe_dict_new(name, NULL);
    } else if (strcmp(name, "svn") == 0) {
        VALUE dictionary, repository, working_copy;
        dictionary = rb_hash_aref(props, CSTR2RVAL("dictionary"));
        repository = rb_hash_aref(props, CSTR2RVAL("repository"));
        working_copy = rb_hash_aref(props, CSTR2RVAL("working_copy"));
        dict = tomoe_dict_new(name,
                              "dictionary", RVAL2TDIC(dictionary),
                              "repository", RVAL2CSTR(repository),
                              "working_copy", RVAL2CSTR(working_copy),
                              NULL);
    } else {
        rb_raise(rb_eArgError, "unknown dictionary type: %s", name);
        dict = NULL;
    }

    return GOBJ2RVALU(dict);
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
    VALUE cTomoeDict;

    cTomoeDict = G_DEF_CLASS(TOMOE_TYPE_DICT, "Dict", mTomoe);

    rb_define_singleton_method(cTomoeDict, "load", td_s_load, 1);
    rb_define_singleton_method(cTomoeDict, "unload", td_s_unload, 0);

    rb_define_singleton_method(cTomoeDict, "new", td_s_new, 2);

    rb_define_method(cTomoeDict, "[]", td_get_char, 1);
    rb_define_method(cTomoeDict, "register", td_register_char, 1);
    rb_define_method(cTomoeDict, "unregister", td_unregister_char, 1);

    rb_define_method(cTomoeDict, "search", td_search, 1);

    rb_define_method(cTomoeDict, "flush", td_flush, 0);
}
